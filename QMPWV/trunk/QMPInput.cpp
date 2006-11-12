//-----------------------------------------------------------------------------
//
// File:	QMPInput.cpp
//
// About:	See QMPInput.h
//
// Authors:	Written by Paul Quinn and Richard Carlson.
//          OO Wrapped by Shao Hao.
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

#include "stdafx.h"
#include "QMPWV.h"
#include "QMPInput.h"

#include "QDecoder.h"
#include "ConfigDlg.h"
#include "AboutDlg.h"

typedef struct _DecoderInfo_t
{
	QDecoder		*pDecoder;
	int				killThread;
	HANDLE			thread_handle;
	CString			playingFile; // UTF8 coding
} DecoderInfo_t;

//..............................................................................
// Static Class Variables
HWND			QMPInput::hwndPlayer;
QCDModInitIn	QMPInput::QCDCallbacks; 

//////////////////////////////////////////////////////////////////////////

DecoderInfo_t decoderInfo; // the unique decoding instance
BOOL	seek_needed = -1;
BOOL	isPaused = 0;
CConfigDlg * g_pdlgCfg;

CString g_utf8String, g_utf8Extensions;

//-----------------------------------------------------------------------------

int QMPInput::Initialize(QCDModInfo *ModInfo, int flags)
{
	char inifile[MAX_PATH];

	if ( !decoderInfo.pDecoder)
		decoderInfo.pDecoder = new QDecoder;

	MBtoUTF8(decoderInfo.pDecoder->GetFullName(),g_utf8String);
	MBtoUTF8(decoderInfo.pDecoder->GetExtensions(),g_utf8Extensions);
	ModInfo->moduleString = (LPTSTR)(LPCTSTR)g_utf8String;
	ModInfo->moduleExtensions = (LPTSTR)(LPCTSTR)g_utf8Extensions;
	ModInfo->moduleCategory = "AUDIO";

	hwndPlayer = (HWND)QCDCallbacks.Service( opGetParentWnd, 0, 0, 0);

	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH, 0);
	g_bUseWVC = GetPrivateProfileInt( "WavPack", "ConfigBit", 0, inifile);

	decoderInfo.thread_handle = INVALID_HANDLE_VALUE;

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void QMPInput::ShutDown(int flags) 
{
	char inifile[MAX_PATH];
	char buf[10];

	Stop( decoderInfo.playingFile, STOPFLAG_FORCESTOP);

	if ( g_pdlgCfg)
		g_pdlgCfg->DestroyWindow();

	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH, 0);
	wsprintf( buf, "%d", g_bUseWVC);
	WritePrivateProfileString( "WavPack", "ConfigBit", buf, inifile);

	if ( !decoderInfo.pDecoder) {
		delete decoderInfo.pDecoder;
		decoderInfo.pDecoder = NULL;
	}
}

//-----------------------------------------------------------------------------

int QMPInput::GetMediaSupported(const char* medianame, MediaInfo *mediaInfo) 
{
	CString filename;
	UTF8toMB(medianame,filename);

	if ( !medianame || !*medianame)
		return FALSE;

	if ( PathIsURL( filename)) {
		if ( mediaInfo) {
			mediaInfo->mediaType = DIGITAL_STREAM_MEDIA;
			mediaInfo->op_canSeek = FALSE; // set false for streaming but reset it later.
		}
		return TRUE; // detail check in Play
	} else {
		if ( filename.ReverseFind( '.') < 0)
			return lstrlen( medianame) > 2; // no extension defaults to me (if not drive letter)

		QDecoder dec;
		if ( dec.IsOurFile( filename)) {
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

int QMPInput::GetTrackExtents(const char* medianame, TrackExtents *ext, int flags)
{
	CString filename;
	UTF8toMB(medianame,filename);

	if ( PathIsURL( filename)) { // for URL address
		ext->track = 1;
		ext->start = 0;
		ext->end = 0;
		ext->bytesize = 0;
		ext->unitpersec = 1000;

		return TRUE; // detail check in Play
	} else {
		QDecoder dec;
		int duration;
		
		if ( (duration = dec.GetDuration( filename)) < 0)
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

int QMPInput::Play(const char* medianame, int playfrom, int playto, int flags)
{
	CString filename;
	UTF8toMB(medianame,filename);

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

		if ( PathIsURL( filename))
            ret = decoderInfo.pDecoder->OpenStream( filename, playfrom);
		else
			ret = decoderInfo.pDecoder->OpenFile( filename);

		if ( ret != 1) {
			delete decoderInfo.pDecoder;
			decoderInfo.pDecoder = NULL;

			return ret;
		}

		// create decoding thread
		DWORD thread_id;

		seek_needed = playfrom > 0 ? playfrom : -1;
		decoderInfo.killThread = 0;
		decoderInfo.playingFile = medianame;

		decoderInfo.thread_handle = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)DecodeThread, &decoderInfo, 0, &thread_id);
		SetThreadPriority( decoderInfo.thread_handle, THREAD_PRIORITY_HIGHEST);

		return TRUE;
	}
}

//-----------------------------------------------------------------------------

int QMPInput::Stop(const char* medianame, int flags)
{
	if ( medianame && *medianame && decoderInfo.playingFile.CompareNoCase( medianame) == 0)
	{
		QCDCallbacks.toPlayer.OutputStop(flags);

		decoderInfo.killThread = 1;
		if ( WaitForSingleObject( decoderInfo.thread_handle, 2000) == WAIT_TIMEOUT) {
//			MessageBox(hwndPlayer, "thread kill timeout", "debug", 0);
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

int QMPInput::Pause(const char* medianame, int flags)
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

int QMPInput::Eject(const char* medianame, int flags)
{
	//
	// TODO : Eject media.
	// flags: 0 = toggle eject state, 1 = ensure open, 2 = ensure closed
	// This will only be called if plugin supports CD Audio
	//

	return TRUE;
}

//-----------------------------------------------------------------------------

void QMPInput::SetEQ(EQInfo *eqinfo)
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

void QMPInput::SetVolume(int levelleft, int levelright, int flags)
{
	QCDCallbacks.toPlayer.OutputSetVol(levelleft, levelright, flags);
}

//-----------------------------------------------------------------------------

int QMPInput::GetCurrentPosition(const char* medianame, long *track, long *offset)
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

void QMPInput::Configure(int flags)
{
	if ( !g_pdlgCfg) {
		g_pdlgCfg = new CConfigDlg;
		g_pdlgCfg->Create( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0), (LPARAM)&g_pdlgCfg);
	}
	g_pdlgCfg->ShowWindow( SW_SHOW);
}

//-----------------------------------------------------------------------------

void QMPInput::About(int flags)
{
	CAboutDlg dlgAbout;

	dlgAbout.DoModal( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0));
}

//-----------------------------------------------------------------------------

IQCDMediaDecoder * QMPInput::CreateDecoderInstance(const WCHAR * medianame, int flags)
{
	char filename[MAX_PATH];

	UCS2toMB( medianame, filename, MAX_PATH);

	return new QDecoder(filename);
}

//-----------------------------------------------------------------------------

DWORD WINAPI QMPInput::DecodeThread(LPVOID lpParameter)
{
	DecoderInfo_t * decoderInfo = (DecoderInfo_t *)lpParameter;
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
			if ( !outputOpened)
				QCDCallbacks.Service( opSetTrackSeekable, (void *)(LPCTSTR)decoderInfo->playingFile, decoderInfo->pDecoder->IsSeekable(), 0);

			// decoding
			if ( !decoderInfo->pDecoder->Decode( &wd))
				done = 1;

			// open output
			if ( !done && !outputOpened) {
				WAVEFORMATEX wf;
				decoderInfo->pDecoder->GetWaveFormFormat( &wf);

				if ( !QCDCallbacks.toPlayer.OutputOpen( decoderInfo->playingFile, &wf)) {
					MessageBox( hwndPlayer, "Error: Open playback plug-in failed!", "Decoder plug-in error", MB_ICONINFORMATION);
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

