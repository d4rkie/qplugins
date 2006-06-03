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

#include "stdafx.h"

#include "QMPdAPDecMgr.h"

#include "ConfigDlg.h"
#include "AboutDlg.h"

#include <set>
#include <algorithm>
using namespace std;

HINSTANCE		hInstance;
HWND			hwndPlayer;
QCDModInitIn	QCDCallbacks;

vector< dAPDecMod > g_Modules;
set< CString > g_Caches;

DecoderInfo_t decoderInfo;

CString g_strPluginDir;
CString g_strCacheFile;
CString g_strExtensions;

BOOL	seek_needed = -1;
BOOL	isPaused = 0;

DWORD WINAPI DecodeThread(LPVOID lpParameter);

CConfigDlg * g_pdlgCfg;
CAboutDlg g_dlgAbout;


//////////////////////////////////////////////////////////////////////////

bool _write_cache_file(const CString & ini)
{
	TCHAR buf[10];

	for ( ITMod it = g_Modules.begin(); it != g_Modules.end(); ++it) {
		CString fn = PathFindFileName( it->strFilePath);

		WritePrivateProfileString( fn, "Desc", it->strDesc, ini);
		WritePrivateProfileString( fn, "Exts", it->strExts, ini);
		ZeroMemory( buf, 10);
		wsprintf( buf, "%d", it->uOrder);
		WritePrivateProfileString( fn, "Order", buf, ini);
		WritePrivateProfileString( fn, "Enabled", it->bEnabled?"TRUE":"FALSE", ini);
	}

	return true;
}

bool _read_cache_file(const CString & ini)
{
	TCHAR buf[200];
	char * p;
	ZeroMemory( buf, 200);

	GetPrivateProfileSectionNames( buf, 200, ini);
	if ( buf[0] == '\0') // no section cached
		return false;

	g_Caches.clear();
	p = buf;
	while ( *p) {
		CString fn(p);
		fn.MakeUpper();
		g_Caches.insert( fn);
		p += (lstrlen(p)+1);
	}

	return !g_Caches.empty();
}

bool _load_from_cache(dAPDecMod & mod, const unsigned int & cnt, const CString & ini)
{
	if ( g_Caches.empty())
		return false;

	CString fn = PathFindFileName(mod.strFilePath);

	CString filename = fn;
	filename.MakeUpper();
	if ( g_Caches.find( filename) != g_Caches.end()) {
		TCHAR buf[200];

		ZeroMemory( buf, 200);
		GetPrivateProfileString( fn, "Desc", "", buf, 200, ini);
		mod.strDesc = buf;

		ZeroMemory( buf, 200);
		GetPrivateProfileString( fn, "Exts", "", buf, 200, ini);
		mod.strExts = buf;

		mod.uOrder = GetPrivateProfileInt( fn, "Order", cnt, ini);

		ZeroMemory( buf, 200);
		GetPrivateProfileString( fn, "Enabled", "TRUE", buf, 200, ini);
		mod.bEnabled = (CString(buf).Find( "TRUE") >= 0);

		return true;
	} else {
		return false;
	}
}

bool _load_one_dll(dAPDecMod & mod, const unsigned int & cnt)
{
	if ( mod.strFilePath.IsEmpty())
		return false;

	HMODULE hmod = LoadLibrary( mod.strFilePath);
	if ( !hmod)
		return false;

	try {
		mod.WhatFormats = (LPWhatFormats)GetProcAddress( hmod, "WhatFormats");
		if ( mod.WhatFormats) {
			int pos;
			char format[10];
			char description[100];

			ZeroMemory( format, 10);
			ZeroMemory( description, 100);

			// get description
			mod.WhatFormats( 10000, format, description);
			if ( lstrlen( description))
				mod.strDesc = description;
			else
				throw(false);

			// get support format
			pos = 0;
			mod.WhatFormats( pos, format, description);
			mod.strExts.Empty();
			while ( lstrlen( format)) {
				if ( !mod.strExts.IsEmpty()) mod.strExts += ":";

				mod.strExts += format;

				mod.WhatFormats( ++pos, format, description);
			}
			if ( mod.strExts.IsEmpty()) throw(false);
			mod.strExts.Remove( '.');
			mod.strExts.MakeUpper();
		} else {
			throw(false);
		}

		mod.CreateANewDecoderObject = (LPCreateANewDecoderObject)GetProcAddress( hmod, "CreateANewDecoderObject");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.DeleteADecoderObject = (LPDeleteADecoderObject)GetProcAddress( hmod, "DeleteADecoderObject");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.GetTrackInfo = (LPGetTrackInfo)GetProcAddress( hmod, "GetTrackInfo");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.OutputWantsABlockOfData = (LPOutputWantsABlockOfData)GetProcAddress( hmod, "OutputWantsABlockOfData");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.ShowAboutOptionsPage = (LPShowAboutOptionsPage)GetProcAddress( hmod, "ShowAboutOptionsPage");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.SkipTo = (LPSkipTo)GetProcAddress( hmod, "SkipTo");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.SetVolume = (LPSetVolume)GetProcAddress( hmod, "SetVolume");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.Pause = (LPPause)GetProcAddress( hmod, "Pause");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.UnPause = (LPUnPause)GetProcAddress( hmod, "UnPause");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.GetStringInfo = (LPGetStringInfo)GetProcAddress( hmod, "GetStringInfo");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.SetIDTagElement = (LPSetIDTagElement)GetProcAddress( hmod, "SetIDTagElement");
		if ( !mod.CreateANewDecoderObject)
			throw(false);
		mod.GetIDTagElement = (LPGetIDTagElement)GetProcAddress( hmod, "GetIDTagElement");
		if ( !mod.GetIDTagElement)
			throw(false);

		mod.uOrder = cnt;
		mod.bEnabled = TRUE;
		throw(true);
	} catch (bool success) {
		if ( success) {
			mod.hMod = hmod;
			return true;
		} else {
			FreeLibrary( hmod);
			return false;
		}
	}
}

bool _load_all_dlls(const char * pfldr)
{
	unsigned int cnt;
	CFindFile ff;
	TCHAR fullpath[MAX_PATH];

	lstrcpy( fullpath, pfldr);
	PathAppend( fullpath, "*.dll");
	if ( !ff.FindFile( fullpath))
		return false;
	// load dBpowerAmp's decoder plug-in
	cnt = 0;
	do {
		dAPDecMod mod;

		mod.hMod = NULL;
		_free_one_dll( mod); // initialize
		mod.strFilePath = ff.GetFilePath();

		if ( _load_from_cache( mod, cnt, g_strCacheFile) || _load_one_dll( mod, cnt)) {
			g_Modules.push_back( mod);
			++cnt;
		}
		// next dll file
	} while ( ff.FindNextFile());

    return !g_Modules.empty();
}

bool _free_one_dll(dAPDecMod & mod)
{
	if ( mod.hMod) {
		if ( mod.hMod) FreeLibrary( mod.hMod);
		mod.hMod = NULL;

		mod.strExts.Empty();

		mod.WhatFormats = NULL;
		mod.CreateANewDecoderObject = NULL;
		mod.DeleteADecoderObject = NULL;
		mod.GetTrackInfo = NULL;
		mod.OutputWantsABlockOfData = NULL;
		mod.ShowAboutOptionsPage = NULL;
		mod.SkipTo = NULL;
		mod.SetVolume = NULL;
		mod.Pause = NULL;
		mod.UnPause = NULL;
		mod.GetStringInfo = NULL;
		mod.SetIDTagElement = NULL;
		mod.GetIDTagElement = NULL;

		mod.bEnabled = FALSE;

		return true;
	} else {
		return false;
	}
}

bool _free_all_dlls(void)
{
	for_each( g_Modules.begin(), g_Modules.end(), _free_one_dll);

	g_Modules.clear();

	return true;
}

bool _is_supported(const CString & inFN, LISTModIT & outLIST)
{
	bool found;

	CString ext = PathFindExtension( inFN)+1;
	if ( ext.IsEmpty())
		return false;

	ext.MakeUpper();
	found = false;
	for ( ITMod it = g_Modules.begin(); it != g_Modules.end(); ++it) {
		if ( (*it).bEnabled && !(*it).strExts.IsEmpty()) {
			int s = 0;
			int e = (*it).strExts.Find( ':', s);
			while ( e > 0) {
				if ( (*it).strExts.Mid( s, e-s) == ext) {
					outLIST.push_back( it);
					found = true;
					break;
				} else {
					s = e+1;
					e = (*it).strExts.Find( ':', s);
				}
			}
			if ( s < (*it).strExts.GetLength()) {
				if ( (*it).strExts.Right( (*it).strExts.GetLength()-s) == ext) {
					outLIST.push_back( it);
					found = true;
				}
			}
		}
	}

	return found;
}

bool _op_compare(const dAPDecMod & L, const dAPDecMod & R)
{
	return (L.uOrder < R.uOrder);
}

void _sort_plugins(void)
{
	stable_sort( g_Modules.begin(), g_Modules.end(), _op_compare);

	// make sure there is no reduplicate order number
	UINT num = 0;
	for ( ITMod it = g_Modules.begin(); it != g_Modules.end(); ++it) {
		it->uOrder = num++;
	}
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
	QCDCallbacks.toModule.SetEQ				= NULL; //SetEQ;
	QCDCallbacks.toModule.SetVolume			= SetVolume;

	return &QCDCallbacks;
}

int Initialize(QCDModInfo *ModInfo, int flags)
{
	char inifile[MAX_PATH], loadpath[MAX_PATH];

	ModInfo->moduleString = "QPlugins dBpowerAMP Decoder Manager v1.0";

	hwndPlayer = (HWND)QCDCallbacks.Service(opGetParentWnd, 0, 0, 0);
	QCDCallbacks.Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	// load cache file
	QCDCallbacks.Service( opGetPluginFolder, loadpath, MAX_PATH, 0);
	PathAppend( loadpath, "dAPplugins.ini");
	g_strCacheFile = loadpath;
	_read_cache_file( g_strCacheFile);

	g_strExtensions = "";

	GetPrivateProfileString(TEXT("dAPplugins Manager"), TEXT("LoadPath"), "", loadpath, MAX_PATH, inifile);
	if ( PathFileExists( loadpath)) {
		_load_all_dlls( loadpath);
		_sort_plugins();

		g_strPluginDir = loadpath;

		for ( ITMod it = g_Modules.begin(); it != g_Modules.end(); ++it) {
			if ( !g_strExtensions.IsEmpty()) g_strExtensions += ":";
			g_strExtensions += it->strExts;
		}
	}
	
	ModInfo->moduleExtensions = (LPTSTR)(LPCTSTR)g_strExtensions;
	ModInfo->moduleCategory = "AUDIO";

	decoderInfo.thread_handle = INVALID_HANDLE_VALUE;
	
	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags)
{
	char inifile[MAX_PATH];

	Stop( decoderInfo.playingFile, flags);

	if ( decoderInfo.pDecoder)
		decoderInfo.DecMod.DeleteADecoderObject(decoderInfo.pDecoder);

	QCDCallbacks.Service( opGetPluginFolder, inifile, MAX_PATH, 0);
	PathAppend( inifile, "dAPplugins.ini");
	_write_cache_file( inifile);

	_free_all_dlls();

	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH, 0);
	WritePrivateProfileString( TEXT("dAPplugins Manager"), TEXT("LoadPath"), g_strPluginDir, inifile);
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
		return FALSE; // detail check in Play
	} else {
		if ( !strrchr( medianame, '.'))
			return (lstrlen( medianame) > 2); // no extension defaults to me (if not drive letter)

		LISTModIT mylist;
		if ( _is_supported(medianame, mylist)) {
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
	LISTModIT mylist;

	if ( !strrchr( medianame, '.'))
		return FALSE;

	if ( _is_supported(medianame, mylist)) {
		int bps, freq, chn, len, size, year, pref;
		char artist[100], track[100], album[100], genre[100];
		ITMod it = mylist.front();

		// Load plug-in when necessary
		if ( (*it).hMod == NULL && !_load_one_dll( (*it), it-g_Modules.begin()))
			return FALSE;

		// use the first available plug-in to parse the track info
		(*it).GetTrackInfo( (char *)medianame, bps, freq, chn, len, size, artist, track, album, year, pref, genre);
		ext->track = 1;
		ext->start = 0;
		ext->end = (UINT)len;
		ext->bytesize = (UINT)size;
		ext->unitpersec = 1000;

		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------

int Play(const char* medianame, int playfrom, int playto, int flags)
{
	if ( !decoderInfo.playingFile.IsEmpty() && decoderInfo.playingFile.CompareNoCase( medianame) != 0) {
		Stop( decoderInfo.playingFile, STOPFLAG_PLAYDONE);
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
		if ( !_is_supported( medianame, decoderInfo.supModules))
			return PLAYSTATUS_UNSUPPORTED;

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
	if ( medianame && *medianame && decoderInfo.playingFile.CompareNoCase( medianame) == 0) {
		QCDCallbacks.toPlayer.OutputStop(flags);
		QCDCallbacks.toPlayer.PlayStopped( decoderInfo.playingFile);

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

int Pause(const char* medianame, int flags)
{
	if ( decoderInfo.pDecoder) {
		switch ( flags)
		{
		case PAUSE_ENABLED:
			{
				isPaused = PAUSE_ENABLED;
				decoderInfo.DecMod.Pause( decoderInfo.pDecoder);
			}

			break;
		case PAUSE_DISABLED:
			{
				isPaused = PAUSE_DISABLED;
				decoderInfo.DecMod.UnPause( decoderInfo.pDecoder);
			}

			break;
		default:
			break;
		}

		return QCDCallbacks.toPlayer.OutputPause(flags);
	} else {
		return FALSE;
	}
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
	if ( decoderInfo.pDecoder)
		decoderInfo.DecMod.SetVolume( decoderInfo.pDecoder, levelleft, levelright);
	else
		QCDCallbacks.toPlayer.OutputSetVol( levelleft, levelright, flags);
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

	return FALSE;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	if ( !g_pdlgCfg) {
		g_pdlgCfg = new CConfigDlg;
		g_pdlgCfg->Create( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0), (LPARAM)&g_pdlgCfg);
	}
	g_pdlgCfg->ShowWindow( SW_SHOW);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	g_dlgAbout.DoModal( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0));
}

//-----------------------------------------------------------------------------

DWORD WINAPI DecodeThread(LPVOID lpParameter)
{
	DecoderInfo_t *decoderInfo = (DecoderInfo_t*)lpParameter;
	BOOL done = FALSE, updatePos = FALSE, outputOpened = FALSE;

	AudioInfo ai;
	WriteDataStruct wd;
	int bps, freq, chn, len;

	WAVEFORMATEX *pwfx;
	bool success = false;
	// find the first available module for decoding
	while ( !decoderInfo->supModules.empty()) {
		bool stream;
		bool proected;
		void * d;
		ENOpenResult result;

		ITMod it = decoderInfo->supModules.front();
		// Load plug-in when necessary
		if ( (*it).hMod == NULL && !_load_one_dll( (*it), it-g_Modules.begin())) {
			decoderInfo->supModules.pop_front();
			continue;
		}

		d = (*it).CreateANewDecoderObject( (LPTSTR)(LPCTSTR)decoderInfo->playingFile, result, stream, &pwfx, proected);
		if ( d && result == OR_OK) {
			decoderInfo->pDecoder = d; // save decoder pointer
			decoderInfo->DecMod = (*it); // save decoder module
			success = true;
			break;
		}

		decoderInfo->supModules.pop_front();
	}
	if ( !success) {
		MessageBox( hwndPlayer, "ERROR: failed on opening file!", "ERROR", MB_OK);
		QCDCallbacks.toPlayer.PlayStopped( decoderInfo->playingFile);
		done = 1;
	} else { // get audio info
		int size, year, pref;
		char artist[100], track[100], album[100], genre[100];

		decoderInfo->DecMod.GetTrackInfo( (LPTSTR)(LPCTSTR)decoderInfo->playingFile, bps, freq, chn, len, size, artist, track, album, year, pref, genre);
		ZeroMemory( &ai, sizeof(AudioInfo));
		ai.struct_size = sizeof(AudioInfo);
		ai.bitrate = bps*1000;
		ai.frequency = freq;
		ai.mode = (chn==2)?0:(chn>2)?4:3;
		CString ext = PathFindExtension(decoderInfo->playingFile)+1;
		ext.MakeUpper();
		lstrcpy( ai.text, ext);
		QCDCallbacks.Service( opSetAudioInfo, &ai, sizeof(AudioInfo), 0);

		pwfx->cbSize = 0;
		if ( !QCDCallbacks.toPlayer.OutputOpen( decoderInfo->playingFile, pwfx)) {
			MessageBox( hwndPlayer, "Error: Open playback plug-in failed!", "Decoder plug-in error", MB_ICONINFORMATION);
			QCDCallbacks.toPlayer.PlayStopped( decoderInfo->playingFile);
			done = TRUE; // cannot open sound device
		}
	}

	const DWORD bufferLen = 13230*8;//576 * bps;
	char * pBuffer = new char[bufferLen];
	// decoding loop
	while ( !decoderInfo->killThread) {
		if ( !done && seek_needed >= 0) { /********************** SEEK ************************/
			QCDCallbacks.toPlayer.OutputFlush( 0);

			decoderInfo->DecMod.SkipTo( decoderInfo->pDecoder, (DWORD)(seek_needed*100./len+.5));

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
			// decoding
			bool isEOF;
			ENDecodeResult result;
			DWORD position;
			DWORD totalLen;

			DWORD bytelen = decoderInfo->DecMod.OutputWantsABlockOfData( decoderInfo->pDecoder, pBuffer, bufferLen, isEOF, result, position, totalLen);
			if ( result != DR_OK) {
				done = 1;
				continue;
			} else {
				if ( bytelen == 0) { // wait for next decoding
					Sleep(10);
					continue;
				}

				wd.bps = pwfx->wBitsPerSample;
				wd.bytelen = bytelen;
				wd.data = pBuffer;
				wd.markerstart = position;
				wd.markerend = 0;
				wd.nch = chn;
				wd.numsamples = wd.bytelen/(wd.bps/8)/wd.nch;
				wd.srate = freq;
			}

			if ( updatePos) {
				QCDCallbacks.toPlayer.PositionUpdate( position);
				updatePos = 0;
			}

			// send data to output
			if ( !QCDCallbacks.toPlayer.OutputWrite( &wd))
				done = 1;

			done = isEOF;
		}

		// catch pause
		while ( isPaused && !decoderInfo->killThread)
			Sleep(50);
	}

	if ( pBuffer)
		delete [] pBuffer;

	// close up
	decoderInfo->thread_handle = INVALID_HANDLE_VALUE;

	if ( decoderInfo->pDecoder) {
		decoderInfo->DecMod.DeleteADecoderObject( decoderInfo->pDecoder);
		decoderInfo->pDecoder = NULL;
		decoderInfo->supModules.clear();
	}

	return 0;
}

