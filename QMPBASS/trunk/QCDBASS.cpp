//-----------------------------------------------------------------------------
//
// File:	QCDBASS.cpp
//
// About:	BASS Sound System Plug-in for Quintessential Player
//
// Authors: Shao Hao, Toke Noer
//
//	QCD multimedia player application Software Development Kit Release 1.0.
//
//	Copyright (C) 1997-2002 Quinnware
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------
// Bugs
// : The bass plugin retaining the file open when you press the stop button
// : Volume in system mode with 32bit is very low/not mirrored to backspeakers
//     (Not looked into yet)

// Feature Reqeusts
// : Add FLAC support via addon
// : Shoutcast stream titles. Currently only the songname tag is supported,
//-----------------------------------------------------------------------------
// 11-05-05 : Added ID3v2 tag reader
// 11-05-05 : Extra preamp slider for files with RG
// 11-05-05 : Fixed CloseHandle(thread_handle) failed
// 05-04-05 : Added VorbisComment reader for replaygain
// 04-04-05 : Fixed EQ wasn't set before eq was changed
// 18-03-05 : Fixed wrong replaygain value for files with no RG info
//-----------------------------------------------------------------------------

#include "QCDBASS.h"

// ConStream dLog;			// Debug output stream

#include "mmreg.h"
#include <memory>

#include "BASSCfgUI.h"


HINSTANCE		hInstance;
HWND			hwndPlayer;
QCDModInitIn	QCDCallbacks;

DecoderInfo_t decoderInfo;

DOUBLE	seek_to			= -1;
BOOL	encoding		= FALSE;
BOOL	paused			= FALSE;
INT64	decode_pos_ms	= 0;

DWORD WINAPI __stdcall DecodeThread(void *b);	// decoding thread only for decode mode
DWORD WINAPI __stdcall PlayThread(void *b);		// playing thread only for system mode

cfg_int uPrefPage("QCDBASS", "PrefPage", 0);	// pref page number
cfg_int xPrefPos("QCDBASS", "xPrefPos", 200);	// left side of property sheet
cfg_int yPrefPos("QCDBASS", "yPrefPos", 300);	// left side of property sheet

cfg_string strExtensions("QCDBASS", "Extensions", "WAV:MP3:MP2:MP1:OGG:MO3:XM:MOD:S3M:IT:MTM", MAX_PATH); // for supported extensions
cfg_int uDeviceNum("QCDBASS", "DeviceNum", 0); // 0=decode, 1...=use internal output module
cfg_int bUse32FP("QCDBASS", "Use32FP", TRUE); // enable 32-bit floating-point sample resolution
cfg_int uPriority("QCDBASS", "Priority", 2); // thread priority, 0=normal, 1=higher, 2=highest, 3=critcal
cfg_int bEqEnabled("QCDBASS", "EQEnabled", TRUE); // enable internal equalizer
cfg_int bShowVBR("QCDBASS", "ShowVBR", TRUE); // display VBR bitrate
cfg_int nSampleRate("QCDBASS", "SampleRate", 44100); // Output sample rate

cfg_int uFadeIn("QCDBASS", "FadeIn", 1000); // fade-in sound
cfg_int uFadeOut("QCDBASS", "FadeOut", 3000); // fade-out sound

cfg_int nPreAmp("QCDBASS", "PreAmp", 0); // preamp
cfg_int nRGPreAmp("QCDBASS", "RGPreAmp", 0); // preamp
cfg_int bHardLimiter("QCDBASS", "HardLimiter", 0); // 6dB hard limiter
cfg_int bDither("QCDBASS", "Dither", 0); // dither
cfg_int uNoiseShaping("QCDBASS", "NoiseShaping", 1); // noise shaping
cfg_int uReplayGainMode("QCDBASS", "ReplayGainMode", 0); // replaygain mode

cfg_int uBufferLen("QCDBASS", "BufferLen", 10); // stream buffer lengthe in milliseconds
cfg_int bStreamTitle("QCDBASS", "StreamTitle", TRUE); // enable stream title

cfg_int bStreamSaving("QCDBASS", "StreamSaving", FALSE); // enable stream saving
cfg_string strStreamSavingPath("QCDBASS", "StreamSavingPath", "", MAX_PATH); // path to saving streamed files
cfg_int bAutoShowStreamSavingBar("QCDBASS", "AutoShowStreamSavingBar", TRUE); // auto show stream saving bar
cfg_int bSaveStreamsBasedOnTitle("QCDBASS", "SaveStreamsBasedOnTitle", TRUE); // save streams based on stream title

cfg_int xStreamSavingBar("QCDBASS", "xStreamSavingBar", 0); // left side of stream saving bar
cfg_int yStreamSavingBar("QCDBASS", "yStreamSavingBar", 0); // top side of stream saving bar

HWND hwndConfig = NULL; // config property sheet
HWND hwndAbout = NULL; // about dialog box
HWND hwndStreamSavingBar = NULL; // stream saving bar

UINT uCurDeviceNum = uDeviceNum;

void show_error(const char *message,...)
{
	char foo[512];
	va_list args;
	va_start(args, message);
	vsprintf(foo, message, args);
	va_end(args);
	MessageBox(hwndPlayer, foo, "BASS Sound System Error", MB_ICONSTOP);
}

void insert_menu(void)
{
	QCDCallbacks.Service(opSetPluginMenuItem, (void *)hInstance, 0, (long)("BASS Plug-in"));
	QCDCallbacks.Service(opSetPluginMenuItem, (void *)hInstance, ID_PLUGINMENU_SETTINGS, (long)("Settings"));
	QCDCallbacks.Service(opSetPluginMenuItem, (void *)hInstance, ID_PLUGINMENU_ENABLE_STREAM_SAVING, (long)("Enable stream saving"));
	QCDCallbacks.Service(opSetPluginMenuState, (void *)hInstance, ID_PLUGINMENU_ENABLE_STREAM_SAVING, bStreamSaving ? MF_CHECKED : MF_UNCHECKED);
	QCDCallbacks.Service(opSetPluginMenuItem, (void *)hInstance, ID_PLUGINMENU_SHOW_STREAM_SAVING_BAR, (long)("Show stream saving bar"));
	QCDCallbacks.Service(opSetPluginMenuState, (void *)hInstance, ID_PLUGINMENU_SHOW_STREAM_SAVING_BAR, hwndStreamSavingBar ? MF_CHECKED : MF_UNCHECKED);
}

void remove_menu(void)
{
	QCDCallbacks.Service(opSetPluginMenuItem, (void *)hInstance, 0, 0);
}

void reset_menu(void)
{
	remove_menu();

	insert_menu();
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) 
{
	if (uMsg == BFFM_INITIALIZED) 
	{
		lpData = (LPARAM)(LPCTSTR)strStreamSavingPath;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);	
	}
	return 0;
}

int browse_folder(LPTSTR pszFolder, LPCTSTR lpszTitle, HWND hwndOwner)
{
	BROWSEINFO		bi;
	LPMALLOC		SHMalloc;
	LPITEMIDLIST	pidlBrowse;
	CHAR			findname[MAX_PATH];
	BOOL			ret = FALSE;

	if (FAILED(SHGetMalloc(&SHMalloc)))         
		return 0;

	bi.hwndOwner = hwndOwner; 
	bi.pidlRoot = NULL;     
	bi.pszDisplayName = NULL; 
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = BIF_NEWDIALOGSTYLE; 
	bi.lpfn = BrowseCallbackProc; 
	bi.lParam = (LPARAM)(LPCWSTR)pszFolder;

	pidlBrowse = SHBrowseForFolder(&bi);     
	if (pidlBrowse != NULL) 
	{
		if (SHGetPathFromIDList(pidlBrowse, findname)) 
		{
			lstrcpy(pszFolder, findname);
			ret = TRUE;
		}

		SHMalloc->Free(pidlBrowse);
	}

	SHMalloc->Release();
	return ret;
}

//-----------------------------------------------------------------------------

int WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		hInstance = hInst;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------

PLUGIN_API QCDModInitIn* INPUTDLL_ENTRY_POINT(QCDModInitIn *ModInit, QCDModInfo *ModInfo)
{
	QCDCallbacks.size						= sizeof(QCDModInitIn);
	QCDCallbacks.version					= PLUGIN_API_VERSION;
	QCDCallbacks.toModule.Initialize		= Initialize;
	QCDCallbacks.toModule.ShutDown			= ShutDown;
	QCDCallbacks.toModule.GetTrackExtents	= GetTrackExtents;
	QCDCallbacks.toModule.GetMediaSupported	= GetMediaSupported;
	QCDCallbacks.toModule.Play				= Play;
	QCDCallbacks.toModule.Pause				= Pause;
	QCDCallbacks.toModule.Stop				= Stop;
	QCDCallbacks.toModule.About				= About;
	QCDCallbacks.toModule.Configure			= Configure;
	QCDCallbacks.toModule.SetEQ				= bEqEnabled ? SetEQ : NULL;;
	QCDCallbacks.toModule.SetVolume			= SetVolume;

	return &QCDCallbacks;
}

int Initialize(QCDModInfo *ModInfo, int flags)
{
	hwndPlayer = (HWND)QCDCallbacks.Service(opGetParentWnd, 0, 0, 0);
	decoderInfo.playingFile = NULL;
	decoderInfo.thread_handle = INVALID_HANDLE_VALUE;

	// load config
	char inifile[MAX_PATH];
	QCDCallbacks.Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	uPrefPage.load(inifile);
	xPrefPos.load(inifile);
	yPrefPos.load(inifile);
	strExtensions.load(inifile);
	uDeviceNum.load(inifile);
	bUse32FP.load(inifile);
	uPriority.load(inifile);
	bEqEnabled.load(inifile);
	bShowVBR.load(inifile);
	nSampleRate.load(inifile);
	uFadeIn.load(inifile);
	uFadeOut.load(inifile);
	nPreAmp.load(inifile);
	nRGPreAmp.load(inifile);
	bHardLimiter.load(inifile);
	bDither.load(inifile);
	uNoiseShaping.load(inifile);
	uReplayGainMode.load(inifile);
	uBufferLen.load(inifile);
	bStreamTitle.load(inifile);
	bStreamSaving.load(inifile);
	strStreamSavingPath.load(inifile);
	bAutoShowStreamSavingBar.load(inifile);
	bSaveStreamsBasedOnTitle.load(inifile);
	xStreamSavingBar.load(inifile);
	yStreamSavingBar.load(inifile);

	ModInfo->moduleString = "BASS Sound System "PLUGIN_VERSION;
	ModInfo->moduleExtensions = (LPTSTR)(LPCTSTR)strExtensions;
	ModInfo->moduleCategory = "AUDIO";

	// insert plug-in menu
	insert_menu();

	// dLog.OpenConsole();

	// Init BASS
	if (create_bass(uDeviceNum)) {
		uCurDeviceNum = uDeviceNum;
		return TRUE;
	}
	else
		return FALSE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags)
{
	Stop(decoderInfo.playingFile, STOPFLAG_FORCESTOP);

	destroy_bass();

	// save config
	char inifile[MAX_PATH];
	QCDCallbacks.Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	uPrefPage.save(inifile);
	xPrefPos.save(inifile);
	yPrefPos.save(inifile);
	strExtensions.save(inifile);
	uDeviceNum.save(inifile);
	bUse32FP.save(inifile);
	uPriority.save(inifile);
	bEqEnabled.save(inifile);
	bShowVBR.save(inifile);
	nSampleRate.save(inifile);
	uFadeIn.save(inifile);
	uFadeOut.save(inifile);
	nRGPreAmp.save(inifile);
	nPreAmp.save(inifile);
	bHardLimiter.save(inifile);
	bDither.save(inifile);
	uNoiseShaping.save(inifile);
	uReplayGainMode.save(inifile);
	uBufferLen.save(inifile);
	bStreamTitle.save(inifile);
	bStreamSaving.save(inifile);
	strStreamSavingPath.save(inifile);
	bAutoShowStreamSavingBar.save(inifile);
	bSaveStreamsBasedOnTitle.save(inifile);
	xStreamSavingBar.save(inifile);
	yStreamSavingBar.save(inifile);

	// remove plug-in menu
	remove_menu();

	if(hwndConfig) {
		DestroyWindow(hwndConfig);
		hwndConfig = NULL;
	}
	if(hwndAbout) {
		DestroyWindow(hwndAbout);
		hwndAbout = NULL;
	}
	if(hwndStreamSavingBar) {
		DestroyWindow(hwndStreamSavingBar);
		hwndStreamSavingBar = NULL;
	}

	// dLog.CloseConsole();
}

//-----------------------------------------------------------------------------

int GetMediaSupported(const char* medianame, MediaInfo *mediaInfo)
{
	if (!medianame || !*medianame)
		return FALSE;

	if ( (PathIsURL(medianame) && strnicmp(medianame, "uvox://", 7)) || // support url but no for AAC stream
		(lstrlen(medianame) > 2 && StrStrI(strExtensions, PathFindExtension(medianame)+1)) ) { // or local file but not driver (Maybe CD latter)

		// Copy from bass constructor, but no need to create bass object + file_reader
		if ( PathIsURL(medianame) ) {
			mediaInfo->mediaType = DIGITAL_STREAM_MEDIA;
			mediaInfo->op_canSeek = strrchr(medianame, '.') > strrchr(medianame, '/'); // internet files are also seekable
		}
		else {
			mediaInfo->mediaType = DIGITAL_FILE_MEDIA;
			mediaInfo->op_canSeek = true;
		}

		return TRUE;
	}
	else
		return FALSE;
}

//-----------------------------------------------------------------------------

int GetTrackExtents(const char* medianame, TrackExtents *ext, int flags)
{
	std::auto_ptr<bass> p_info(new bass(medianame));

	if ( p_info->init(false) ) {
		ext->track = 1;
		ext->start = 0;
		ext->end = (UINT)p_info->get_length() * 1000;
		ext->bytesize = (UINT)p_info->get_size();
		ext->unitpersec = 1000;

		return TRUE;
	}
	
	return FALSE;
}

//-----------------------------------------------------------------------------

int Play(const char* medianame, int playfrom, int playto, int flags)
{
	encoding = flags & PLAYFLAG_ENCODING;

	if (decoderInfo.playingFile && lstrcmpi(decoderInfo.playingFile, medianame)) {
		Stop(decoderInfo.playingFile, STOPFLAG_PLAYDONE);
	}

	if (!decoderInfo.playingFile) {
		if (uCurDeviceNum != uDeviceNum) { // reinitialization our bass when device number changed
			if ( destroy_bass() && create_bass(uDeviceNum) )
				uCurDeviceNum = uDeviceNum;
			else
				return PLAYSTATUS_FAILED;
		}

		if (decoderInfo.pDecoder)
			delete decoderInfo.pDecoder;
		decoderInfo.pDecoder = new bass(medianame, uCurDeviceNum == 0, !!bUse32FP);
		if (!decoderInfo.pDecoder)
			return PLAYSTATUS_FAILED;

		if (!decoderInfo.pDecoder->is_stream()) { // local file should be initialized now, but stream should do this in thread later avoiding blocking
			int ret = decoderInfo.pDecoder->init();

			if ( ret != 1 ) {
				if (decoderInfo.pDecoder) {
					delete decoderInfo.pDecoder;
					decoderInfo.pDecoder = NULL;
				}

				return ret;
			}
		}

		decoderInfo.playingFile = strdup(medianame);
	}

	decoderInfo.killThread = 0;
	seek_to = (double)playfrom;

	SetEQ(NULL);

	if (paused)
		Pause(decoderInfo.playingFile, PAUSE_DISABLED);

	if (decoderInfo.thread_handle != INVALID_HANDLE_VALUE)
		return PLAYSTATUS_SUCCESS;

	if ( (decode_pos_ms != seek_to) && !decoderInfo.pDecoder->seek(seek_to) ) {
		delete decoderInfo.pDecoder; decoderInfo.pDecoder = NULL;
		if (decoderInfo.playingFile) {
			free(decoderInfo.playingFile);
			decoderInfo.playingFile = NULL;
		}

		return PLAYSTATUS_FAILED;
	}

	decode_pos_ms = playfrom;
	seek_to = -1;

	// create decoding thread
	DWORD thread_id;

	decoderInfo.thread_handle = CreateThread(NULL, 0, 
		decoderInfo.pDecoder->is_decode ? (LPTHREAD_START_ROUTINE)DecodeThread : (LPTHREAD_START_ROUTINE)PlayThread, 
		(void*)&decoderInfo, 0, &thread_id); // create thread for decoding or playing
	SetThreadPriority(decoderInfo.thread_handle, encoding ? 0 : (uPriority == 3 ? THREAD_PRIORITY_TIME_CRITICAL : uPriority) );

	return PLAYSTATUS_SUCCESS;
}

//-----------------------------------------------------------------------------

int Stop(const char* medianame, int flags)
{
	if ( medianame && *medianame && !lstrcmpi(decoderInfo.playingFile, medianame) ) {
		if (decoderInfo.pDecoder->is_decode) // stop output first for decoding
			QCDCallbacks.toPlayer.OutputStop(flags);

		// destroy play/decoding thread
		decoderInfo.killThread = 1;
		if (decoderInfo.thread_handle != INVALID_HANDLE_VALUE) {
			if (WaitForSingleObject(decoderInfo.thread_handle, 10000) == WAIT_TIMEOUT)
				TerminateThread(decoderInfo.thread_handle, 0);

			CloseHandle(decoderInfo.thread_handle);
			decoderInfo.thread_handle = INVALID_HANDLE_VALUE;			
		}

		decoderInfo.pDecoder->stop(flags); // stop all

		// clear & reset all
		if (decoderInfo.playingFile) {
			free(decoderInfo.playingFile);
			decoderInfo.playingFile = NULL;
		}
		if (decoderInfo.pDecoder) {
			delete decoderInfo.pDecoder;
			decoderInfo.pDecoder = NULL;
		}
		decoderInfo.info.reset();

		seek_to = -1;
		decode_pos_ms = 0;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------

int Pause(const char* medianame, int flags)
{
	if (!decoderInfo.pDecoder) return FALSE;

	paused = flags;

	if (decoderInfo.pDecoder->is_decode) {
		decoderInfo.pDecoder->pause(flags); // just set timer

		if ( QCDCallbacks.toPlayer.OutputPause(flags) )
			return TRUE;
	}
	else {
		if (decoderInfo.pDecoder->pause(flags))
			return TRUE;
	}

	paused = !flags;
	return FALSE;
}

//-----------------------------------------------------------------------------

int Eject(const char* medianame, int flags)
{
	//
	// TODO : Eject media.
	// flags: 0 = toggle eject state, 1 = ensure open, 2 = ensure closed
	// This will only be called if plugin supports CD Audio
	//

	return TRUE;
}

//-----------------------------------------------------------------------------

void SetEQ(EQInfo *eqinfo)
{
	EQInfo eq;
	if (!eqinfo) {
		QCDCallbacks.Service(opGetEQVals, &eq, 0, 0);
		eqinfo = &eq;
	}

	if (decoderInfo.pDecoder)	// Running, so update
		decoderInfo.pDecoder->set_eq(bEqEnabled && eqinfo->enabled, eqinfo->bands);
}

//-----------------------------------------------------------------------------

void SetVolume(int levelleft, int levelright, int flags)
{
	if ( !decoderInfo.pDecoder->set_volume((levelleft+levelright)/2) )
        QCDCallbacks.toPlayer.OutputSetVol(levelleft, levelright, flags);
}

//-----------------------------------------------------------------------------

int GetCurrentPosition(const char* medianame, long *track, long *offset)
{
	*track = 1;
	*offset = (long)decode_pos_ms;

	return TRUE;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	switch(flags)
	{
	case ID_PLUGINMENU_ENABLE_STREAM_SAVING:
		{
			bStreamSaving = !bStreamSaving;

			if (hwndStreamSavingBar)
				SendMessage(hwndStreamSavingBar, WM_INITDIALOG, 0, 0);

			// modify our menu item status
			reset_menu();
		}

		break;
	case ID_PLUGINMENU_SHOW_STREAM_SAVING_BAR:
		{
			if(hwndStreamSavingBar) {
				DestroyWindow(hwndStreamSavingBar);
				hwndStreamSavingBar = NULL;
			}
			else
				hwndStreamSavingBar = DoStreamSavingBar(hInstance, NULL);

			reset_menu();
		}

		break;
	default:
		{
			if(!hwndConfig)
				hwndConfig = DoConfigSheet(hInstance, hwndPlayer);
		}

		break;
	}
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	if(!hwndAbout)
		hwndAbout = DoAboutDlg( hInstance, (HWND)QCDCallbacks.Service(opGetPropertiesWnd, NULL, 0, 0) );
}

//-----------------------------------------------------------------------------

// used for convert 32-bit floating-data to 16-bit data. For Vis only now
int cnv32to16(void *buffer, const int size)
{
	short *out_buffer = (short *)(buffer);
	char *in_buffer = (char *)(buffer);

	for(int i = 0; i < size; i += 4) {
		short v = (short)(*(float *)(in_buffer + i) * 0x7fff);

		if(v > 0x7fff) v = 0x7fff;
		else if (v < -0x8000) v = -0x8000;

		*out_buffer++ = v;
	}

	return size/2;
}

//-- playing thread only for system mode
DWORD WINAPI __stdcall PlayThread(void *b)
{
	DecoderInfo_t *decoderInfo = (DecoderInfo_t*)b;
	BOOL done = FALSE, updatePos = FALSE;

	if (decoderInfo->pDecoder->is_stream() && 
		(!decoderInfo->pDecoder->set_stream_buffer_length(uBufferLen * 1000) || 
		!decoderInfo->pDecoder->init()) ) { // stream should be initialized here
			QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile);

			return 0;
	}

	// OK, play it
	if (!decoderInfo->pDecoder->play()) {
		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile);
		return 0;
	}

		// audio specs
	unsigned int samplerate		= (unsigned int)(decoderInfo->pDecoder->get_srate());
	unsigned int bitspersample	= (unsigned int)(decoderInfo->pDecoder->get_bps());
	unsigned int numchannels	= (unsigned int)(decoderInfo->pDecoder->get_nch());
	unsigned int lengthMS		= (unsigned int)decoderInfo->pDecoder->get_length() * 1000;
	unsigned int avgbitrate		= (unsigned int)(decoderInfo->pDecoder->get_bitrate());

	if ( numchannels <= 0 || samplerate <= 0 /*|| bitrate <= 0 */) {
		show_error("Error: invalid media format!");
		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile);
		done = TRUE;
	}

	const DWORD VIS_BUFFER_SIZE = 2048*numchannels*(bitspersample/8); // get 576 samples as winamp does, maybe safest^_^
	// alloc decoding buffer
	LPBYTE pVisData = NULL;
	if (!done) {
		pVisData = new BYTE[VIS_BUFFER_SIZE];

		if (pVisData == NULL) {
			show_error("Error: Out of memory!");
			QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile);
			done = TRUE;
		}
	}

	while (!decoderInfo->killThread) {
		/********************** SEEK ************************/
		if (!done && seek_to >= 0) {
			double ms = seek_to;
			seek_to = -1;
			updatePos = 1;

			if (decoderInfo->pDecoder->seek(ms)) {
				decode_pos_ms = (__int64)ms;
				QCDCallbacks.toPlayer.OutputFlush((unsigned int)ms);
			}
			else
				done = TRUE;
		}

		/********************* QUIT *************************/
		if (done) { // only avaliable when playdone or output error
			if (seek_to < 0) {
				decoderInfo->pDecoder->stop(STOPFLAG_PLAYDONE);
				QCDCallbacks.toPlayer.PlayDone(decoderInfo->playingFile);
			}
			else if (seek_to >= 0) {
				done = FALSE;
				continue;
			}
			break;
		}

		/******************* UPDATE INFO ****************/
		else {
			AudioInfo ai;
			ai.struct_size = sizeof(AudioInfo);
			ai.frequency = samplerate;
			ai.mode = (numchannels == 2) ? 0 : 3;
			ai.text[0] = '\0';

			const char *type = decoderInfo->pDecoder->get_type();
			if ( lstrcmpi(type, "mp1") == 0 ) {
				ai.level = 1;
				ai.layer = 1;
			}
			else if ( lstrcmpi(type, "mp2") == 0 ) {
				ai.level = 1;
				ai.layer = 2;
			}
			else if ( lstrcmpi(type, "mp3") == 0 ) {
				ai.level = 1;
				ai.layer = 3;
			}
			else
				lstrcpyn(ai.text, type, sizeof(ai.text));

			// update bitrate info
			ai.bitrate = bShowVBR ? decoderInfo->pDecoder->get_bitrate() : avgbitrate;

			QCDCallbacks.Service(opSetAudioInfo, &ai, sizeof(AudioInfo), 0);

			// playback
			decode_pos_ms = (unsigned long)(decoderInfo->pDecoder->get_current_time());

			int out_size = VIS_BUFFER_SIZE;
			decoderInfo->pDecoder->get_data(pVisData, &out_size);
			if (bitspersample == 32)
				out_size = cnv32to16(pVisData, out_size);

			WriteDataStruct wd;
			wd.bytelen = out_size;
			wd.data = pVisData;
			wd.markerend = 0;
			wd.markerstart = WAVE_VIS_DATA_ONLY;
			wd.bps = 16; // always 16-bit for Vis
			wd.nch = numchannels;
			wd.numsamples = wd.bytelen / (wd.nch*wd.bps/8);
			wd.srate = samplerate;
			QCDCallbacks.toPlayer.OutputWrite(&wd);

			if (decoderInfo->pDecoder->is_playing()) {
				QCDCallbacks.toPlayer.PositionUpdate((unsigned int)decode_pos_ms);

				Sleep(50); // slow down, not to fast:)
			}
			else
				done = TRUE;
		}
		// catch pause
		while (paused && !decoderInfo->killThread) Sleep(50);
	}

	// Clean up

	if (pVisData)
		delete [] pVisData;
	return 0;
}

//-- decoding thread
DWORD WINAPI __stdcall DecodeThread(void *b)
{
	DecoderInfo_t *decoderInfo = (DecoderInfo_t*)b;
	BOOL done = FALSE, updatePos = FALSE;

	if (decoderInfo->pDecoder->is_stream() && 
		(!decoderInfo->pDecoder->set_stream_buffer_length(uBufferLen * 1000) || 
		!decoderInfo->pDecoder->init()) ) { // stream should be initialized here

		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile);
		return 0;
	}

	// audio specs
	unsigned int samplerate		= (unsigned int)(decoderInfo->pDecoder->get_srate());
	unsigned int bitspersample	= (unsigned int)(decoderInfo->pDecoder->get_bps());
	unsigned int numchannels	= (unsigned int)(decoderInfo->pDecoder->get_nch());
	unsigned int lengthMS		= (unsigned int)(decoderInfo->pDecoder->get_length() * 1000);
	unsigned int avgbitrate		= (unsigned int)(decoderInfo->pDecoder->get_bitrate());

	if ( numchannels <= 0 || samplerate <= 0 /*|| bitrate <= 0 */) {
		show_error("Error: invalid media format!");
		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile);
		done = TRUE;
	}

	// open output
	if (!done) {
		WAVEFORMATEX wf;
		wf.wFormatTag = decoderInfo->pDecoder->get_format();
		wf.cbSize = 0;
		wf.nChannels = numchannels;
		wf.wBitsPerSample = bDither ? 16 : bitspersample;
		wf.nSamplesPerSec = samplerate;
		wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
		wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
		if (!QCDCallbacks.toPlayer.OutputOpen(decoderInfo->playingFile, &wf)) {
			show_error("Error: Failed openning output plugin!");
			QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile);
			done = TRUE; // cannot open sound device
		}
	}

	const DWORD BUFFER_SIZE = 576*numchannels*(bitspersample/8); // get 576 samples as winamp does, maybe safest^_^
	// alloc decoding buffer
	HANDLE hHeap = NULL;
	HANDLE pRawData = NULL;
	if (!done) {
		hHeap = HeapCreate(0, BUFFER_SIZE, BUFFER_SIZE);
		if (hHeap)
			pRawData = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, BUFFER_SIZE);
        
        if (hHeap == NULL || pRawData == NULL) {
			show_error("Error: Out of memory!");
			QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile);
			done = TRUE;
		}
	}

	while (!decoderInfo->killThread) {
		/********************** SEEK ************************/
		if (!done && seek_to >= 0) {
			double ms = seek_to;
			seek_to = -1;
			updatePos = 1;

			if (decoderInfo->pDecoder->seek(ms)) {
				decode_pos_ms = (__int64)ms;
				QCDCallbacks.toPlayer.OutputFlush((unsigned int)ms);
			}
			else
				done = TRUE;
		}

		/********************* QUIT *************************/
		if (done) { // only avaliable when playdone or output error
			if (QCDCallbacks.toPlayer.OutputDrain(0) && seek_to < 0) {
				QCDCallbacks.toPlayer.OutputStop(STOPFLAG_PLAYDONE);
				QCDCallbacks.toPlayer.PlayDone(decoderInfo->playingFile);
			}
			else if (seek_to >= 0) {
				done = FALSE;
				continue;
			}
			break;
		}

		/******************* DECODE TO BUFFER ****************/
		else {
			static int ct = 51; // timer counter to slow display
			if (ct++ > 50) {
				AudioInfo ai;
				ai.struct_size = sizeof(AudioInfo);
				ai.frequency = samplerate;
				ai.mode = (numchannels == 2) ? 0 : 3;
				ai.text[0] = '\0';

				const char *type = decoderInfo->pDecoder->get_type();
				if ( lstrcmpi(type, "mp1") == 0 ) {
					ai.level = 1;
					ai.layer = 1;
				}
				else if ( lstrcmpi(type, "mp2") == 0 ) {
					ai.level = 1;
					ai.layer = 2;
				}
				else if ( lstrcmpi(type, "mp3") == 0 ) {
					ai.level = 1;
					ai.layer = 3;
				}
				else
					lstrcpyn(ai.text, type, sizeof(ai.text));

				// update bitrate info
				ai.bitrate = bShowVBR ? decoderInfo->pDecoder->get_bitrate() : avgbitrate;

				QCDCallbacks.Service(opSetAudioInfo, &ai, sizeof(AudioInfo), 0);
				ct = 0; // reset counter
			}

            // decode
			int out_size = BUFFER_SIZE;
			DWORD ret;

			ret = decoderInfo->pDecoder->decode(pRawData, &out_size);
			if (ret == 0 || ret == -1) // error or reach the eof
				done = TRUE;

			if (!decoderInfo->killThread && ret > 0) {
				// send to output
				WriteDataStruct wd;

				decode_pos_ms = (unsigned long)(decoderInfo->pDecoder->get_current_time());

				if (out_size) {
					wd.bytelen = out_size;
					wd.data = pRawData;
					wd.markerend = 0;
					wd.markerstart = (unsigned int)decode_pos_ms;
					wd.bps = bDither ? 16 : bitspersample;
					wd.nch = numchannels;
					wd.numsamples = wd.bytelen / (wd.nch*wd.bps/8);
					wd.srate = samplerate;

					if (!QCDCallbacks.toPlayer.OutputWrite(&wd))
						done = TRUE;
				}
			}

			if (updatePos) {
				QCDCallbacks.toPlayer.PositionUpdate((unsigned int)decode_pos_ms);
				updatePos = 0;
			}
		}
		// catch pause
		while (paused && !decoderInfo->killThread)
			Sleep(50);

		Sleep(encoding ? 0 : 5);
	}

	// Clean up
	if (pRawData) HeapFree(hHeap, 0, pRawData);
	if (hHeap) HeapDestroy(hHeap);

	return 0;
}