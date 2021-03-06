//-----------------------------------------------------------------------------
//
// File:	QCDBASS.cpp
//
// About:	BASS Sound System Plug-in for Quintessential Player
//
// Authors: Shao Hao, Toke Noer, Ted Hess
//
//  Quintessential Player Plugin Development Kit
//
//	Copyright (C) 1997-2008 Quinnware
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
// : Encoding to wav doesn't work (TAH: Should work - not tested)
//     (Tokelil: Talked to Paul about this, and he will add float support to the wav encoding plug-in)
//
//-----------------------------------------------------------------------------
// 09-02-09 : Fixed stream saving filename errors (normalize names for Windows)
// 07-10-09 : Fixed track name tag update when playing Shoutcast streams
// 04-26-08 : General cleanup, including:
//				- Tighter thread control and Window destruction 
//				- Fixed volume control in BASS playback.
//				- Added ASCII comments tag parsing
//				- Fixed crash on unload - really!
// 04-14-08 : Fix support of QMP Library import. Added QCDFileInfo interface
// 04-13-08 : Updated to BASS v2.4. Fix crash on exit, cleanup add-on exts
// 03-22-08 : Stream Saver UI rework and fixes. Fixed some minor bugs
// 09-21-06 : Getting ready for last release for QCD
//              - Code cleaning, configuration dialog cleaning
//              - Automatic import of addons extension support
//              - resize_reservoir() to avoid constant alloc/dealloc in decode
// 08-29-06 : Add 24bit resolution support.
// 04-06-06 : Updated to support BASS v2.3.0.0
// 02-25-06 : 1. Fixed stream title display bug (for QCD only).
//            2. New mechanism of destroying config sheet.
// 02-02-05 : Fixed adding of .1 files etc. (+IsExtentionSupported())
// 10-10-05 : Added add-ons support based on BASS v2.2
// 11-05-05 : Added ID3v2 tag reader
// 11-05-05 : Extra preamp slider for files with RG
// 11-05-05 : Fixed CloseHandle(thread_handle) failed
// 05-04-05 : Added VorbisComment reader for replaygain
// 04-04-05 : Fixed EQ wasn't set before eq was changed
// 18-03-05 : Fixed wrong replaygain value for files with no RG info
//-----------------------------------------------------------------------------

#pragma warning(disable:4786)		// Disable STL list warnings
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES  1

#include "QCDBASS.h"
#include "mmreg.h"
#include <memory>
#include "BASSCfgUI.h"

HINSTANCE		hInstance;
HWND			hwndPlayer;
QCDModInitIn	QCDCallbacks;

DecoderInfo_t	decoderInfo;

INT64	seek_to			= -1;
BOOL	encoding		= FALSE;
BOOL	paused			= FALSE;
INT64	decode_pos_ms	= 0;

DWORD WINAPI __stdcall DecodeThread(void *b);		// decoding thread only for decode mode
DWORD WINAPI __stdcall PlayThread(void *b);			// playing thread only for system mode

cfg_int uPrefPage("QCDBASS", "PrefPage", 0);		// pref page number
cfg_int xPrefPos("QCDBASS", "xPrefPos", 200);		// left side of property sheet
cfg_int yPrefPos("QCDBASS", "yPrefPos", 300);		// left side of property sheet

cfg_int uDeviceNum("QCDBASS", "DeviceNum", 0);		// 0=decode, 1...=use internal output module
cfg_int uResolution("QCDBASS", "Resolution", 32);    // 16=16bit, 24=24bit, 32=32bit(floating point)
cfg_int bDither("QCDBASS", "Dither", 0);			// dither
cfg_int uNoiseShaping("QCDBASS", "NoiseShaping", 1);// noise shaping
cfg_int uPriority("QCDBASS", "Priority", 2);		// thread priority, 0=normal, 1=higher, 2=highest, 3=critcal
cfg_int bEqEnabled("QCDBASS", "EQEnabled", TRUE);	// enable internal equalizer
cfg_int bShowVBR("QCDBASS", "ShowVBR", TRUE);		// display VBR bitrate
cfg_int nSampleRate("QCDBASS", "SampleRate", 44100);// Output sample rate

cfg_int uFadeIn("QCDBASS", "FadeIn", 100);			// fade-in sound
cfg_int uFadeOut("QCDBASS", "FadeOut", 300);		// fade-out sound

cfg_int nPreAmp("QCDBASS", "PreAmp", 0);			// preamp
cfg_int nRGPreAmp("QCDBASS", "RGPreAmp", 0);		// preamp
cfg_int bHardLimiter("QCDBASS", "HardLimiter", 0);	// 6dB hard limiter
cfg_int uReplayGainMode("QCDBASS", "ReplayGainMode", 0);// replaygain mode

cfg_int uBufferLen("QCDBASS", "BufferLen", 10);			// stream buffer lengthen in milliseconds
cfg_int bStreamTitle("QCDBASS", "StreamTitle", TRUE);	// enable stream title

cfg_int bStreamSaving("QCDBASS", "StreamSaving", FALSE);// enable stream saving
cfg_int bStreamSaveBarVisible("QCDBASS", "StreamSavingVisible", FALSE);// stream saving dialog
cfg_string strStreamSavingPath("QCDBASS", "StreamSavingPath", "", MAX_PATH);  // path to saving streamed files
cfg_int bAutoShowStreamSavingBar("QCDBASS", "AutoShowStreamSavingBar", TRUE); // auto show stream saving bar
cfg_int bSaveStreamsBasedOnTitle("QCDBASS", "SaveStreamsBasedOnTitle", TRUE); // save streams based on stream title

cfg_int xStreamSavingBar("QCDBASS", "xStreamSavingBar", 0);		// left side of stream saving bar
cfg_int yStreamSavingBar("QCDBASS", "yStreamSavingBar", 0);		// top side of stream saving bar

cfg_string strAddonsDir("QCDBASS", "AddonsDir", "", MAX_PATH);	// path of addons' directory

cfg_string  strExtensions("QCDBASS", "Extensions", "MP3:OGG:WAV:MP2:MP1:AIFF:MO3:XM:MOD:S3M:IT:MTM", MAX_PATH); // for supported extensions
std::string strAddonExtensions = "";
std::string strAllExtensions = "";

HWND hwndConfig = NULL;				// config property sheet
HWND hwndAbout = NULL;				// about dialog box
HWND hwndStreamSavingBar = NULL;	// stream saving bar

list<std::string> listAddons;
list<std::string> listExtensions;

UINT uCurDeviceNum = -1;		// None selected yet

void reset_decoder();

//-----------------------------------------------------------------------------

int WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		hInstance = hInst;
		DisableThreadLibraryCalls(hInst);
	}

	if (fdwReason == DLL_PROCESS_DETACH) {
		_ASSERTE(hwndConfig == NULL);
		_ASSERTE(hwndStreamSavingBar == NULL);

		while (hwndStreamSavingBar) {
			MSG msg;
			if (!PeekMessage(&msg, hwndStreamSavingBar, 0, 0, PM_REMOVE))
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
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
	QCDCallbacks.toModule.Eject				= NULL;
	QCDCallbacks.toModule.About				= About;
	QCDCallbacks.toModule.Configure			= Configure;
	QCDCallbacks.toModule.SetEQ				= bEqEnabled ? SetEQ : NULL;
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
	uResolution.load(inifile);
	if ( uResolution > 24) // disable dither for 32bit
		bDither = FALSE;
	else if ( uResolution < 24)
		bDither.load(inifile);
	else
		bDither = TRUE; // enable dither for 24bit always
	uNoiseShaping.load(inifile);
	uPriority.load(inifile);
	bEqEnabled.load(inifile);
	bShowVBR.load(inifile);
	nSampleRate.load(inifile);
	uFadeIn.load(inifile);
	uFadeOut.load(inifile);
	nPreAmp.load(inifile);
	nRGPreAmp.load(inifile);
	bHardLimiter.load(inifile);
	uReplayGainMode.load(inifile);
	uBufferLen.load(inifile);
	bStreamTitle.load(inifile);
	bStreamSaving.load(inifile);
	bStreamSaveBarVisible.load(inifile);
	strStreamSavingPath.load(inifile);
	bAutoShowStreamSavingBar.load(inifile);
	bSaveStreamsBasedOnTitle.load(inifile);
	xStreamSavingBar.load(inifile);
	yStreamSavingBar.load(inifile);
	
	strAddonsDir.load(inifile);
	char path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	PathRemoveFileSpec(path);
	if(strAddonsDir.is_empty()) // set to default (plug-ins directory)
		lstrcpy((LPTSTR)(LPCTSTR)strAddonsDir, path);

	listAddons.clear();
	listExtensions.clear();


	// Init BASS
	if (create_bass(uDeviceNum, hwndPlayer))
		uCurDeviceNum = uDeviceNum;
	else
		return FALSE;

	strAllExtensions = strAddonExtensions;
	strAllExtensions.append(":");
	strAllExtensions.append(strExtensions);

	ModInfo->moduleString      = "QPlugins BASS Sound System "PLUGIN_VERSION;
	ModInfo->moduleExtensions  = (LPSTR)strAllExtensions.c_str();
	ModInfo->moduleCategory    = "AUDIO";

	// Create stream saving bar
	hwndStreamSavingBar = CreateDialogIndirect(hInstance, LoadResDialog(hInstance, IDD_STREAM_SAVING_BAR), NULL, (DLGPROC)StreamSavingBarProc);
	ShowStreamSavingBar(bStreamSaveBarVisible);

	// insert plug-in menu
	insert_menu();

	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags)
{
	Stop(decoderInfo.playingFile, STOPFLAG_SHUTDOWN);

	// Remove About dialog if active
	if(hwndAbout) {
		DestroyWindow(hwndAbout);
		hwndAbout = NULL;
	}

	// Close config window to update pref/location vars
	if(hwndConfig) {
		SendMessage(hwndConfig, WM_CLOSE, 0, 0);
	}

	// Remove SSBar and get location, etc.
	if(hwndStreamSavingBar) {
		SendMessage(hwndStreamSavingBar, WM_CLOSE, 0, 0);
	}

	// save config
	char inifile[MAX_PATH];
	QCDCallbacks.Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	uPrefPage.save(inifile);
	xPrefPos.save(inifile);
	yPrefPos.save(inifile);
	strExtensions.save(inifile);
	uDeviceNum.save(inifile);
	uResolution.save(inifile);
	bDither.save(inifile);
	uNoiseShaping.save(inifile);
	uPriority.save(inifile);
	bEqEnabled.save(inifile);
	bShowVBR.save(inifile);
	nSampleRate.save(inifile);
	uFadeIn.save(inifile);
	uFadeOut.save(inifile);
	nRGPreAmp.save(inifile);
	nPreAmp.save(inifile);
	bHardLimiter.save(inifile);
	uReplayGainMode.save(inifile);
	uBufferLen.save(inifile);
	strAddonsDir.save(inifile);
	bStreamTitle.save(inifile);
	bStreamSaving.save(inifile);
	bStreamSaveBarVisible.save(inifile);
	strStreamSavingPath.save(inifile);
	bAutoShowStreamSavingBar.save(inifile);
	bSaveStreamsBasedOnTitle.save(inifile);
	xStreamSavingBar.save(inifile);
	yStreamSavingBar.save(inifile);

	// remove plug-in menu
	remove_menu();

	destroy_bass();
}

//-----------------------------------------------------------------------------

bool IsExtensionSupported(const char* strExt)
{
	//OutputDebugString(":IsExtenstionSupported()");

	if (listExtensions.empty()) { // Build the list
		//OutputDebugString(" -- Building extension list:");
		TCHAR* token;
		TCHAR* str;
		
		// Addons
		str = _tcsdup((LPCTSTR)strAddonExtensions.c_str());
		token = _tcstok(str, _T(":")); // Get first token
		while(token != NULL)
		{
			listExtensions.push_back(string(token));
			//OutputDebugString(token);
			token = _tcstok(NULL, _T(":")); // Get next token
		}
		//OutputDebugString(" -- Done addons!");
		free(str);

		str = _tcsdup((LPCTSTR)strExtensions);
		token = _tcstok(str, _T(":")); // Get first token
		while(token != NULL)
		{
			listExtensions.push_back(string(token));
			//OutputDebugString(token);
			token = _tcstok(NULL, _T(":")); // Get next token
		}
		//OutputDebugString(" -- Done!");
		free(str);
	}

	// Search the list
	list<std::string>::const_iterator it;
	for(it = listExtensions.begin(); it != listExtensions.end(); ++it)
	{
		//OutputDebugString(((std::string)*it).c_str());
		if (lstrcmpi(((std::string)*it).c_str(), strExt) == 0)
			return TRUE;		
	}
	
	//OutputDebugString(":IsExtenstionSupported() - return FALSE");
	return FALSE;
}

//-----------------------------------------------------------------------------

int GetMediaSupported(const char* medianame, MediaInfo *mediaInfo)
{
	//OutputDebugString(":GetMediaSupported(): ");
	if (medianame) {
		//OutputDebugString(medianame);

		if (lstrlen(medianame) < 3) // No support for CD drives etc.
			return FALSE;

		if (PathIsURL(medianame)) {
			if (!StrNCmpI(medianame, "uvox://", 7)) {
				// Check if AAC support is loaded
				if (!IsExtensionSupported("AAC"))
					return FALSE; // no support for AAC stream
			}

			mediaInfo->mediaType = DIGITAL_STREAM_MEDIA;
			mediaInfo->op_canSeek = strrchr(medianame, '.') > strrchr(medianame, '/'); // internet files are also seekable
			return TRUE;
		}
		else {
			if (IsExtensionSupported(PathFindExtension(medianame)+1)) {
				mediaInfo->mediaType = DIGITAL_AUDIOFILE_MEDIA;
				mediaInfo->op_canSeek = true;
				return TRUE;
			}
		}
	}

	//OutputDebugString("Not supported!");
	return FALSE;
}

//-----------------------------------------------------------------------------

int GetTrackExtents(const char* medianame, TrackExtents *ext, int flags)
{
	if (medianame) {
		bass p_info(medianame);
		if ( p_info.init(false) )
		{
			ext->track = 1;
			ext->start = 0;
			ext->end = (UINT)p_info.get_length() * 1000;
			ext->bytesize = (UINT)p_info.get_size();
			ext->unitpersec = 1000;

			return TRUE;
		}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------

int Play(const char* medianame, int playfrom, int playto, int flags)
{
	encoding = flags & PLAYFLAG_ENCODING;

	if (medianame && decoderInfo.playingFile && lstrcmpi(decoderInfo.playingFile, medianame)) {
		Stop(decoderInfo.playingFile, STOPFLAG_PLAYSKIP);
	}

	if (!decoderInfo.playingFile) {
		if (uCurDeviceNum != uDeviceNum) { // reinitialization our bass when device number changed
			if ( destroy_bass() && create_bass(uDeviceNum, hwndPlayer) )
				uCurDeviceNum = uDeviceNum;
			else
				return PLAYSTATUS_FAILED;
		}

		// Create new decoder
		if (decoderInfo.pDecoder)
			delete decoderInfo.pDecoder;
		decoderInfo.pDecoder = new bass(medianame, uCurDeviceNum == 0, !!bDither || uResolution == 32);
		if (!decoderInfo.pDecoder)
			return PLAYSTATUS_FAILED;

		// local file should be initialized now, but stream should do this in thread later avoiding blocking
		if (!decoderInfo.pDecoder->is_stream()) {
			int ret = decoderInfo.pDecoder->init();
			// init returns 1 on success
			if ( ret != 1 ) {
				if (decoderInfo.pDecoder) {
					delete decoderInfo.pDecoder;
					decoderInfo.pDecoder = NULL;
				}
				// init returns -1 if BASS_ERROR_FILEFORM (unsupported media)
				return (ret == 0) ? PLAYSTATUS_FAILED : PLAYSTATUS_UNSUPPORTED;
			}
		}

		decoderInfo.playingFile = _strdup(medianame);
	}

	decoderInfo.killThread = 0;
	seek_to = playfrom;

	SetEQ(NULL);

	if (paused)
		Pause(decoderInfo.playingFile, PAUSE_DISABLED);

	if (decoderInfo.thread_handle != INVALID_HANDLE_VALUE)
		return PLAYSTATUS_SUCCESS;

	if ( (decode_pos_ms != seek_to) && !decoderInfo.pDecoder->seek(seek_to) ) {
		reset_decoder();
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
	if ( medianame && decoderInfo.playingFile && !lstrcmpi(decoderInfo.playingFile, medianame) ) {
		if (decoderInfo.pDecoder->is_decode) // stop output first for decoding
			QCDCallbacks.toPlayer.OutputStop(flags);

		// destroy play/decoding thread
		decoderInfo.killThread = 1;
		if (decoderInfo.thread_handle != INVALID_HANDLE_VALUE) {
			if (WaitForSingleObject(decoderInfo.thread_handle, 5000) == WAIT_TIMEOUT) {
				OutputDebugString("*** Forced thread termination!\n");
				TerminateThread(decoderInfo.thread_handle, 0);
			}
			CloseHandle(decoderInfo.thread_handle);
			decoderInfo.thread_handle = INVALID_HANDLE_VALUE;
		}

		decoderInfo.pDecoder->stop(flags); // stop all

		reset_decoder();

		seek_to = -1;
		decode_pos_ms = 0;
		paused = FALSE;
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
	if ( decoderInfo.pDecoder && !decoderInfo.pDecoder->set_volume((levelleft+levelright)/2) )
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

			UpdateSSBarStatus(hwndStreamSavingBar);

			// modify our menu item status
			set_menu_state();
		}

		break;
	case ID_PLUGINMENU_SHOW_STREAM_SAVING_BAR:
		{
			bStreamSaveBarVisible = !bStreamSaveBarVisible;

			ShowStreamSavingBar(bStreamSaveBarVisible);

			set_menu_state();
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
// Menu functions
//-----------------------------------------------------------------------------

void insert_menu(void)
{
	QCDCallbacks.Service(opSetPluginMenuItem,  (void *)hInstance, 0, (long)("BASS Plug-in"));
	QCDCallbacks.Service(opSetPluginMenuItem,  (void *)hInstance, ID_PLUGINMENU_SETTINGS, (long)("Settings"));
	QCDCallbacks.Service(opSetPluginMenuItem,  (void *)hInstance, ID_PLUGINMENU_ENABLE_STREAM_SAVING, (long)("Enable stream saving"));
	QCDCallbacks.Service(opSetPluginMenuItem,  (void *)hInstance, ID_PLUGINMENU_SHOW_STREAM_SAVING_BAR, (long)("Show stream saving bar"));
	set_menu_state();
}

void remove_menu(void)
{
	QCDCallbacks.Service(opSetPluginMenuItem, (void *)hInstance, 0, 0);
}

void set_menu_state(void)
{
	QCDCallbacks.Service(opSetPluginMenuState, (void *)hInstance, ID_PLUGINMENU_ENABLE_STREAM_SAVING, bStreamSaving ? MF_CHECKED : MF_UNCHECKED);
	QCDCallbacks.Service(opSetPluginMenuState, (void *)hInstance, ID_PLUGINMENU_SHOW_STREAM_SAVING_BAR, bStreamSaveBarVisible ? MF_CHECKED : MF_UNCHECKED);
}


//-----------------------------------------------------------------------------
// Misc functions
//-----------------------------------------------------------------------------

void show_error(const char *message,...)
{
	char foo[512];
	va_list args;
	va_start(args, message);
	vsprintf(foo, message, args);
	va_end(args);
	MessageBox(hwndPlayer, foo, "BASS Sound System Error", MB_ICONSTOP);
}

void reset_decoder()
{
	if (decoderInfo.thread_handle != INVALID_HANDLE_VALUE) {
		CloseHandle(decoderInfo.thread_handle);
		decoderInfo.thread_handle = INVALID_HANDLE_VALUE;
	}

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

	return;
}

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
// Decoding callbacks
//-----------------------------------------------------------------------------

//-- playing thread only for system mode
DWORD WINAPI __stdcall PlayThread(void *b)
{
	DecoderInfo_t *decoderInfo = (DecoderInfo_t*)b;
	BOOL done = FALSE, updatePos = FALSE;

	if (decoderInfo->pDecoder->is_stream())
	{
		// init streams here
		decoderInfo->pDecoder->set_stream_buffer_length(uBufferLen * 1000);
		if (decoderInfo->pDecoder->init() != 1)
		{ 
			// BASS failed to recognize stream
			QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile, PLAYSTOPPED_DEFAULT);
			reset_decoder();
			return 0;
		}
	}

	// OK, play it
	if (!decoderInfo->pDecoder->play()) {
		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile, PLAYSTOPPED_DEFAULT);
		reset_decoder();
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
		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile, PLAYSTOPPED_UNSUPPORTED);
		reset_decoder();
		return 0;
	}

	const DWORD VIS_BUFFER_SIZE = 2048*numchannels*(bitspersample/8); // get 576 samples as winamp does, maybe safest^_^
	// alloc decoding buffer
	LPBYTE pVisData = NULL;
	pVisData = new BYTE[VIS_BUFFER_SIZE];

	if (pVisData == NULL) {
		show_error("Error: Out of memory!");
		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile, PLAYSTOPPED_DEFAULT);
		reset_decoder();
		return 0;
	}

	while (!decoderInfo->killThread) {
		/********************** SEEK ************************/
		if (!done && seek_to >= 0) {
			INT64 ms = seek_to;
			seek_to = -1;
			updatePos = 1;

			if (decoderInfo->pDecoder->seek(ms)) {
				decode_pos_ms = ms;
				QCDCallbacks.toPlayer.OutputFlush((unsigned int)ms);
			}
			else
				done = TRUE;
		}

		/********************* QUIT *************************/
		if (done) { // only available when playdone or output error
			if (seek_to < 0) {
				decoderInfo->pDecoder->stop(STOPFLAG_PLAYDONE);
				QCDCallbacks.toPlayer.PlayDone(decoderInfo->playingFile, STOPFLAG_PLAYDONE);
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
			if (type)
			{
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
			}

			// update bitrate info
			ai.bitrate = bShowVBR ? decoderInfo->pDecoder->get_bitrate() : avgbitrate;

			QCDCallbacks.Service(opSetAudioInfo, &ai, sizeof(AudioInfo), 0);

			// playback
			decode_pos_ms = decoderInfo->pDecoder->get_current_time();

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
				Sleep(50); // There ought to be a better way to conserve CPU!
			}
			else
				done = TRUE;
		}

		// catch pause ( Should be a semaphore )
		while (paused && !decoderInfo->killThread)
			Sleep(50);
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

	if (decoderInfo->pDecoder->is_stream())
	{ 
		// streams initialized here
		decoderInfo->pDecoder->set_stream_buffer_length(uBufferLen * 1000);
		if (decoderInfo->pDecoder->init() != 1)
		{
			// BASS rejected stream
			QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile, PLAYSTOPPED_DEFAULT);
			reset_decoder();
			return 0;
		}
	}

	// audio specs
	unsigned int samplerate		= (unsigned int)(decoderInfo->pDecoder->get_srate());
	//unsigned int bitspersample	= (unsigned int)(decoderInfo->pDecoder->get_bps());
	unsigned int numchannels	= (unsigned int)(decoderInfo->pDecoder->get_nch());
	unsigned int lengthMS		= (unsigned int)(decoderInfo->pDecoder->get_length() * 1000);
	unsigned int avgbitrate		= (unsigned int)(decoderInfo->pDecoder->get_bitrate());

	if ( numchannels <= 0 || samplerate <= 0 /*|| bitrate <= 0 */) {
		show_error("Error: invalid media format!");
		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile, PLAYSTOPPED_UNSUPPORTED);
		reset_decoder();
		return 0;
	}

	// open output
	WAVEFORMATEX wf;
	wf.wFormatTag = decoderInfo->pDecoder->get_format();
	wf.cbSize = 0;
	wf.nChannels = numchannels;
	wf.wBitsPerSample = uResolution;
	wf.nSamplesPerSec = samplerate;
	wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	if (!QCDCallbacks.toPlayer.OutputOpen(decoderInfo->playingFile, &wf)) {
		show_error("Failed opening output device!");
		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile, PLAYSTOPPED_DEFAULT);
		reset_decoder();
		return 0; // cannot open sound device
	}

	const DWORD BUFFER_SIZE = 576*numchannels*((uResolution > 16 ? 32 : 16)/8); // get 576 samples as winamp does, maybe safest^_^

	// alloc decoding buffer
	BYTE *pRawData = NULL;
	pRawData = new BYTE[BUFFER_SIZE];
    
    if (pRawData == NULL) {
		show_error("Error: Out of memory!");
		QCDCallbacks.toPlayer.PlayStopped(decoderInfo->playingFile, PLAYSTOPPED_DEFAULT);
		reset_decoder();
		return 0;
	}

	while (!decoderInfo->killThread) {
		/********************** SEEK ************************/
		if (!done && seek_to >= 0) {
			INT64 ms = seek_to;
			seek_to = -1;
			updatePos = 1;

			if (decoderInfo->pDecoder->seek(ms)) {
				decode_pos_ms = ms;
				QCDCallbacks.toPlayer.OutputFlush((unsigned int)ms);
			}
			else
				done = TRUE;
		}

		/********************* QUIT *************************/
		if (done) { // only avaliable when playdone or output error
			if (QCDCallbacks.toPlayer.OutputDrain(0) && seek_to < 0) {
				QCDCallbacks.toPlayer.OutputStop(STOPFLAG_PLAYDONE);
				QCDCallbacks.toPlayer.PlayDone(decoderInfo->playingFile, STOPFLAG_PLAYDONE);
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
				if (type)
				{
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
				}

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

			if (!decoderInfo->killThread && ret > 0) { // Send to output
				WriteDataStruct wd;

				decode_pos_ms = decoderInfo->pDecoder->get_current_time();

				if (out_size) {
					wd.bytelen		= out_size;
					wd.data			= pRawData;
					wd.markerend	= 0;
					wd.markerstart	= (unsigned int)decode_pos_ms;
					wd.bps			= uResolution;
					wd.nch			= numchannels;
					wd.numsamples	= wd.bytelen / (wd.nch*wd.bps/8);
					wd.srate		= samplerate;

					if (!QCDCallbacks.toPlayer.OutputWrite(&wd))
						done = TRUE;
				}
			}

			if (updatePos) {
				QCDCallbacks.toPlayer.PositionUpdate((unsigned int)decode_pos_ms);
				updatePos = 0;
			}
			decoderInfo->pDecoder->set_stream_title(decode_pos_ms);
		}

		// catch pause (use semaphore?)
		while (paused && !decoderInfo->killThread)
			Sleep(50);
	}

	// Clean up
	if (pRawData) 
		delete []pRawData;

	return 0;
}