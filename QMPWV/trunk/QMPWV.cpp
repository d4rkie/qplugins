//-----------------------------------------------------------------------------
//
// File:	QCDInputDLL.cpp
//
// About:	See QCDInputDLL.h
//
// Authors:	Written by Paul Quinn and Richard Carlson.
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

#include "QInput.h"


HINSTANCE		hInstance;
HWND			hwndPlayer;
QCDModInitIn	QCDCallbacks;


DWORD WINAPI DecodeThread(LPVOID lpParameter);

DecoderInfo_t decoderInfo;

QCfgUI cfgUI;

BOOL	seek_needed = -1;
BOOL	isPaused = 0;

//-----------------------------------------------------------------------------

int WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
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
	QCDCallbacks.toModule.SetEQ				= NULL;//SetEQ;
	QCDCallbacks.toModule.SetVolume			= SetVolume;

	return &QCDCallbacks;
}

int Initialize(QCDModInfo *ModInfo, int flags)
{
	char inifile[MAX_PATH];

	if ( !decoderInfo.pDecoder)
		decoderInfo.pDecoder = new QDecoder;

	ModInfo->moduleString = (LPTSTR)(LPCTSTR)decoderInfo.pDecoder->GetFullName();
	ModInfo->moduleExtensions = (LPTSTR)(LPCTSTR)decoderInfo.pDecoder->GetExtensions();
	ModInfo->moduleCategory = "AUDIO";

	hwndPlayer = (HWND)QCDCallbacks.Service( opGetParentWnd, 0, 0, 0);

	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH, 0);
	cfgUI.SetIniFileName( inifile);
	cfgUI.LoadSettings();

	decoderInfo.thread_handle = INVALID_HANDLE_VALUE;

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	char inifile[MAX_PATH];

	Stop( decoderInfo.playingFile, STOPFLAG_FORCESTOP);

	cfgUI.CloseAll();

	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH, 0);
	cfgUI.SetIniFileName( inifile);
	cfgUI.SaveSettings();
	
	if ( !decoderInfo.pDecoder) {
		delete decoderInfo.pDecoder;
		decoderInfo.pDecoder = NULL;
	}
}

//-----------------------------------------------------------------------------

int GetMediaSupported(const char* medianame, MediaInfo *mediaInfo) 
{
	if (!medianame || !*medianame)
		return FALSE;

	if ( PathIsURL( medianame)) {
		if ( mediaInfo) {
			mediaInfo->mediaType = DIGITAL_STREAM_MEDIA;
			mediaInfo->op_canSeek = FALSE; // set false for streaming but reset it later.
		}
		return TRUE; // detail check in Play
	} else {
		char *ch = strrchr( medianame, '.');
		if (!ch)
			return (lstrlen( medianame) > 2); // no extension defaults to me (if not drive letter)

		QDecoder dec;
		if ( dec.IsOurFile( medianame)) {
			if (mediaInfo) {
				mediaInfo->mediaType = DIGITAL_AUDIOFILE_MEDIA;
				mediaInfo->op_canSeek = TRUE;
			}
			return TRUE;
		}
		return FALSE;
	}
}

//-----------------------------------------------------------------------------

int GetTrackExtents(const char* medianame, TrackExtents *ext, int flags)
{
	if ( PathIsURL( medianame)) { // for url address
		ext->track = 1;
		ext->start = 0;
		ext->end = 0;
		ext->bytesize = 0;
		ext->unitpersec = 1000;

		return TRUE; // detail check in Play
	} else {
		QDecoder dec;
		int duration;
		
		if ( (duration = dec.GetDuration( medianame)) < 0)
			return FALSE;

		ext->track = 1;
		ext->start = 0;
		ext->end = duration;
		ext->bytesize = 0;
		ext->unitpersec = 1000;

		return TRUE;
	}
}

//-----------------------------------------------------------------------------

int Play(const char* medianame, int playfrom, int playto, int flags)
{
	if ( !decoderInfo.playingFile.IsEmpty() && decoderInfo.playingFile.CompareNoCase( medianame) != 0) {
		QCDCallbacks.toPlayer.OutputStop(STOPFLAG_PLAYDONE);
		Stop(decoderInfo.playingFile, STOPFLAG_PLAYDONE);
	}

	if (isPaused) {
		// Update the player controls to reflect the new unpaused state
		QCDCallbacks.toPlayer.OutputPause(0);

		Pause(medianame, 0);

		if (playfrom >= 0)
			seek_needed = playfrom;

		return TRUE;
	} else if (decoderInfo.thread_handle != INVALID_HANDLE_VALUE) { // is playing
		seek_needed = playfrom;
		return TRUE;
	} else {
		int ret;

		if ( !decoderInfo.pDecoder)
			decoderInfo.pDecoder = new QDecoder;
        decoderInfo.pDecoder->Close();

		if ( PathIsURL( medianame))
            ret = decoderInfo.pDecoder->OpenStream( medianame, playfrom);
		else
			ret = decoderInfo.pDecoder->OpenFile( medianame);

		if ( ret != 1)
			return ret;

		// create decoding thread
		DWORD thread_id;

		seek_needed = playfrom > 0 ? playfrom : -1;
		decoderInfo.killThread = 0;
		decoderInfo.playingFile = medianame;

		decoderInfo.thread_handle = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)DecodeThread, (void*)&decoderInfo, 0, &thread_id);
		SetThreadPriority( decoderInfo.thread_handle, THREAD_PRIORITY_HIGHEST);

		return TRUE;
	}
}

//-----------------------------------------------------------------------------

int Stop(const char* medianame, int flags)
{
	if ( medianame && *medianame && decoderInfo.playingFile.CompareNoCase( medianame) == 0)
	{
		QCDCallbacks.toPlayer.OutputStop(flags);

		decoderInfo.killThread = 1;
		if ( WaitForSingleObject( decoderInfo.thread_handle, 2000) == WAIT_TIMEOUT) {
//			MessageBox(hwndPlayer, "ape thread kill timeout", "debug", 0);
//			TerminateThread(decoderInfo.thread_handle, 0);
		}
		CloseHandle( decoderInfo.thread_handle);
		decoderInfo.thread_handle = INVALID_HANDLE_VALUE;

		decoderInfo.playingFile.Empty();
		isPaused = 0;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------

int Pause(const char* medianame, int flags)
{
	isPaused = flags;

	if ( QCDCallbacks.toPlayer.OutputPause(flags)) {
		// send back pause/unpause notification
		return TRUE;
	}

	isPaused = !flags;
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
	//
	// TODO : Update plugin EQ settings
	// Do nothing to ignore
	//
	// Future: removing this functions (setting ModInit->toModule.SetEQ 
	// at entry point to NULL) will tell QCD to use internal generic EQ.
	// So if this plugin doesn't support EQ, remove this function.
	//
}

//-----------------------------------------------------------------------------

void SetVolume(int levelleft, int levelright, int flags)
{
	QCDCallbacks.toPlayer.OutputSetVol(levelleft, levelright, flags);
}

//-----------------------------------------------------------------------------

int GetCurrentPosition(const char* medianame, long *track, long *offset)
{
	//
	// TODO : Gets current playing track and offset - must be as exact
	// and as current as possible. Return TRUE for success.
	// set *track to current track number (always 1 for digital files or streams)
	// set *offset to current offset in track (same units as used in GetTrackExtents)
	//
	// For audio playing through output plugin, call:
	// return QCDCallbacks->toPlayer.OutputGetCurrentPosition((UINT*)offset, 0);
	//

	return QCDCallbacks.toPlayer.OutputGetCurrentPosition((UINT*)offset, 0);
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	cfgUI.Configure( flags);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	cfgUI.About( flags);
}

//-----------------------------------------------------------------------------

DWORD WINAPI DecodeThread(LPVOID lpParameter)
{
	DecoderInfo_t *decoderInfo = (DecoderInfo_t*)lpParameter;
	BOOL done = FALSE, updatePos = FALSE, outputOpened = FALSE;

	AudioInfo ai;
	WriteDataStruct wd;


	// decoding loop
	while ( !decoderInfo->killThread) {
		if ( !done && seek_needed >= 0) { /********************** SEEK ************************/
			QCDCallbacks.toPlayer.OutputFlush( 0);

			if ( !decoderInfo->pDecoder->Seek( seek_needed))
				done = TRUE;

			seek_needed = -1;
			updatePos = 1;
		}

		if ( done) { /********************* QUIT *************************/
			if ( QCDCallbacks.toPlayer.OutputDrain( 0) && !(seek_needed >= 0)) {
				decoderInfo->thread_handle = INVALID_HANDLE_VALUE;
				QCDCallbacks.toPlayer.OutputStop( STOPFLAG_PLAYDONE);
				QCDCallbacks.toPlayer.PlayDone( decoderInfo->playingFile);
			} else if ( seek_needed >= 0) {
				done = FALSE;
				continue;
			}
			break;
		} else { /******************* DECODE TO BUFFER ****************/
			// set track seekable flag
			if ( !done && !outputOpened)
				QCDCallbacks.Service( opSetTrackSeekable, (void *)(LPCTSTR)decoderInfo->playingFile, decoderInfo->pDecoder->IsSeekable(), 0);

			// decoding
			if ( !done) {
				if ( !decoderInfo->pDecoder->Decode( &wd))
					done = 1;
			}

			// open output
			if ( !done && !outputOpened) {
				WAVEFORMATEX wf;
				decoderInfo->pDecoder->GetWaveFormFormat( &wf);

				if ( !QCDCallbacks.toPlayer.OutputOpen( decoderInfo->playingFile, &wf)) {
					MessageBox( hwndPlayer, "Error: Open playback plug-in failed!", "FAAD MPEG4 AAC plug-in", MB_ICONINFORMATION);
					QCDCallbacks.toPlayer.PlayStopped( decoderInfo->playingFile);
					done = TRUE; // cannot open sound device

					continue;
				} else {
					outputOpened = TRUE;
				}
			}

			if (updatePos) {
				QCDCallbacks.toPlayer.PositionUpdate( decoderInfo->pDecoder->GetCurPos());
				updatePos = 0;
			}

			// update audio info
			decoderInfo->pDecoder->GetAudioInfo( &ai);
			QCDCallbacks.Service( opSetAudioInfo, &ai, sizeof(AudioInfo), 0);

			// send data to output
			if (!QCDCallbacks.toPlayer.OutputWrite( &wd))
				done = 1;
		}

		// catch pause
		while ( isPaused && !decoderInfo->killThread)
			Sleep(50);
	}

	// close up
	decoderInfo->thread_handle = INVALID_HANDLE_VALUE;

	if ( decoderInfo->pDecoder) {
		decoderInfo->pDecoder->Close();
		delete decoderInfo->pDecoder;
		decoderInfo->pDecoder = NULL;
	}

	return 0;
}

