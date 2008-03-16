//-----------------------------------------------------------------------------
//
// File:	QMPInput.cpp
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
#include "QMPCUE.h"
#include "QMPInput.h"

#include <atlcoll.h>

#include "QCUESheet.h"

#include "AboutDlg.h"

//-----------------------------------------------------------------------------

WCHAR g_szInputModDisplayStr[1024] = {0};

HWND			QMPInput::hwndPlayer;
QCDModInitIn	QMPInput::QCDCallbacks;

typedef struct __PLUGINCACHE {
	int number;   // loading number
	CPath path;   // plug-in name(a.k.a. module name for "GetModuleHandle")
	CString exts; // supported extensions in form ":.ext1:.ext2:^^^:.extN:"
} PLUGINCACHE;
CAtlList< PLUGINCACHE > g_listPlugins;

typedef QCDModInitIn * (*pfnEntryPoint)();

// for playing architecture
int g_nCurTrack; // current track number
int g_nTotalTracks; // total track counts of current image file
QCDModInitIn * g_qcdCallbacks; // our callback for decoding
CPath g_pathVTrack; // current virtual track file name
CPath g_pathImageFile; // playing image file name
double g_lfStart, g_lfEnd; // the position for playback
TrackExtents g_tePlaying; // TrackExtents of the playing image file

HANDLE g_hPlayStartedEvent; // a new track has started
HANDLE g_hEncodeStoppedEvent; // current track encoding has stopped.

CStringA g_strModulePath; // store Multi-Bytes/UTF-8 string
CStringW g_wstrAgentPath; // store UNICODE string

//-----------------------------------------------------------------------------

/***********************************************************************
 ** Hook the position updating service
 **
 ** fix position update
 ***********************************************************************/
void QMPInput::PositionUpdate(unsigned int position)
{
	if ( (position * 1.0) > g_lfStart)
		position = (unsigned int)(position * 1.0 - g_lfStart);
	QCDCallbacks.toPlayer.PositionUpdate( position);
}

/***********************************************************************
 ** Re-hook the service API.
 **
 ** QMP will invoke the entry pointer once the plug-in calls the OuputOpen.
 ** Then, our hooked APIs will be changed by QMP.
 **
 ** So, we re-hook it again ^_^
 ***********************************************************************/
int QMPInput::OutputOpen(const char* medianame, WAVEFORMATEX* wf)
{
	int ret = QCDCallbacks.toPlayer.OutputOpen( (const char *)(LPCTSTR)g_pathVTrack, wf);

	// re-hook the APIs
	if ( g_qcdCallbacks)
		_hook_player_functions( g_qcdCallbacks);

	return ret;
}

/***********************************************************************
 ** Hook the data output service
 **
 ** Notify the player when we meet the end of each virtual track.
 ***********************************************************************/
int QMPInput::OutputWrite(WriteDataStruct * wd)
{
	if ( wd->markerstart >= g_lfEnd && g_nCurTrack < g_nTotalTracks) { // ready to switch?
		double cur = QCDCallbacks.Service( opGetOutputTime, NULL, 0, 0) / 1000.0 * g_tePlaying.unitpersec;
		if ( cur >= (g_lfEnd - g_lfStart)) { // It's time to switch!
			if ( g_hEncodeStoppedEvent){ // encoding
				if ( WAIT_TIMEOUT == WaitForSingleObject( g_hEncodeStoppedEvent, 0)) {
					// Notify the player that we finished current VT and prepare for a new VT.
					// This will be invoke ONLY ONCE until a new encoding starts.
					QCDCallbacks.toPlayer.PlayDone( (char *)(LPCTSTR)g_pathVTrack, 0); // This will make player invoke "Stop"
					WaitForSingleObject( g_hEncodeStoppedEvent, INFINITE); // Wait until player invoke "Stop"
				}

				return TRUE; // return immediately for encoding
			}
			if ( g_hPlayStartedEvent) { // playback
				ResetEvent( g_hPlayStartedEvent);
				// notify the player that we finished current VT and prepare for a new VT.
				QCDCallbacks.toPlayer.PlayDone( (char *)(LPCTSTR)g_pathVTrack, 0); // This will make player invoke a new "Play"
				WaitForSingleObject( g_hPlayStartedEvent, INFINITE); // Wait until the new g_lfStart is set for playback. AutoReset mode
			}
		}
	}

	if ( (wd->markerstart * 1.0) > g_lfStart)
		wd->markerstart = (unsigned int)(wd->markerstart * 1.0 - g_lfStart);

	return QCDCallbacks.toPlayer.OutputWrite( wd);
}

/***********************************************************************
 ** Hook the Output Stop service
 **
 ** Stop the playing virtual track before stop output.
 ***********************************************************************/
int QMPInput::OutputStop(int flags)
{
	if ( STOPFLAG_SHUTDOWN == flags || STOPFLAG_FORCESTOP == flags)
		QCDCallbacks.toPlayer.PlayStopped( (const char *)(LPCTSTR)g_pathVTrack, flags);

	return QCDCallbacks.toPlayer.OutputStop( flags);
}

/***********************************************************************
 ** Hook the Play Stop service
 **
 ** When plug-in invoke "PlayStopped", stop for virtual track
 ***********************************************************************/
void QMPInput::PlayStopped(const char* medianame, int flags)
{
	QCDCallbacks.toPlayer.PlayStopped( (const char *)(LPCTSTR)g_pathVTrack, flags);
}

/***********************************************************************
 ** Hook the Play done service
 **
 ** fix for last virtual track.
 ** When plug-in invoke "PlayDone", done for virtual track
 ***********************************************************************/
void QMPInput::PlayDone(const char* medianame, int flags)
{
	QCDCallbacks.toPlayer.PlayDone( (const char *)(LPCTSTR)g_pathVTrack, flags);
}

//-----------------------------------------------------------------------------

bool _is_supported(const CString & strExts, const CString & strExt)
{
	CString pattern = _T(":") + strExt + _T(":");
	return strExts.Find( pattern.MakeUpper()) >= 0;
}

/***********************************************************************
 ** Read all input plug-ins from cache file
 **
 ***********************************************************************/
void _read_plugin_cache(const CPath & pathPluginFolder, const CPath & pathCacheFile)
{
	TCHAR myfn[MAX_PATH];

	// get filename of myself
	GetModuleFileName( g_hInstance, myfn, MAX_PATH);

	CPath pattern(pathPluginFolder); pattern.Append( _T("*.dll"));

	CFindFile ff;
	if ( ! ff.FindFile( pattern)) return ;
	do {
		CString fn = ff.GetFileName();

		// skip myself
		if ( 0 == lstrcmpi( ff.GetFilePath(), myfn)) continue;

		// skip any non-input plug-in
		TCHAR value[200];
		if ( 0 == GetPrivateProfileString( fn, _T("Input0"), _T(""), value, 10, pathCacheFile)) continue;
		if ( 0 != _tcsnicmp( value, _T("TRUE"), 4)) continue;

		// get plug-in order number
		int num;
		num = GetPrivateProfileInt( fn, _T("Param20"), 0, pathCacheFile);
		// get the supported extensions and make it in the form of ":.ext1:.ext2:...:.extN:"
		CString exts;
		GetPrivateProfileString( fn, _T("Exts20"), _T(""), value, 200, pathCacheFile);
		exts = value; exts.Replace( _T(":"), _T(":."));
		exts = _T(":.") + exts + _T(":");

		// save input plug-in info.
		PLUGINCACHE pc = { num, ff.GetFilePath(), exts.MakeUpper() };
		g_listPlugins.AddTail( pc);
	} while ( ff.FindNextFile());

	ff.Close();
}

//-----------------------------------------------------------------------------

int QMPInput::Initialize(QCDModInfo *ModInfo, int flags)
{
	TCHAR inifile[MAX_PATH];
	size_t i, j;

	ModInfo->moduleString = (LPSTR)g_szInputModDisplayStr;
	ModInfo->moduleExtensions = (LPSTR)L"VT"; // virtual track

	// load display name
	ResInfo resInfo = { sizeof(ResInfo), g_hInstance, MAKEINTRESOURCE(IDS_INPUT_MODULE), 0, 0 };
	QCDCallbacks.Service( opLoadResString, (void*)g_szInputModDisplayStr, (long)sizeof(g_szInputModDisplayStr), (long)&resInfo);

	// the unique player window handle
	hwndPlayer = (HWND)QCDCallbacks.Service( opGetParentWnd, 0, 0, 0);

	// determine the installation mode
	// Multi-User mode: there is only one plug-in cache file in uers's application settings folder.
	// Single-User mode: each plug-in folder has a plug-in cache file
	QCDCallbacks.Service( opGetPlayerFolder, inifile, MAX_PATH*sizeof(TCHAR), 0);
	PathAppend( inifile, _T("UserPrefs"));
	BOOL bMultiUser = PathFileExists( inifile);

	// clean the plug-in list
	g_listPlugins.RemoveAll();

	// get the default plug-in cache file
	TCHAR cachefile[MAX_PATH];
	QCDCallbacks.Service( opGetPluginCacheFile, cachefile, MAX_PATH*sizeof(TCHAR), 0);

	// read plug-in cache in all plug-in folders
	QCDCallbacks.Service( opGetPlayerSettingsFile, inifile, MAX_PATH*sizeof(TCHAR), 0);
	i = 0;
	do {
		TCHAR pluginfldr[MAX_PATH], buf[20];
		wsprintf( buf, _T("PluginFolder%i"), i++);
		GetPrivateProfileString( _T("Folders"), buf, _T(""), pluginfldr, MAX_PATH, inifile);

		// Finished here
		if ( 0 >= lstrlen( pluginfldr)) break;

		CPath pathCacheFile(pluginfldr); pathCacheFile.Append( _T("PluginCache.ini"));
		if ( bMultiUser) pathCacheFile = cachefile; // multi-user mode, only one cache file
		if ( pathCacheFile.FileExists()) _read_plugin_cache( pluginfldr, pathCacheFile);
	} while ( true);

	// sort the plug-in with their order number
	size_t count = g_listPlugins.GetCount();
	for ( j = 0; j < count; ++j) {
		for( i = 0; i < count-1; ++i) {
			POSITION l = g_listPlugins.FindIndex(i);
			int ln = g_listPlugins.GetAt(l).number;
			POSITION r = g_listPlugins.FindIndex(i+1);
			int rn = g_listPlugins.GetAt(r).number;
			if ( ln > rn) g_listPlugins.SwapElements( l, r);
		}
	}

	// initialize all global variables
	g_nCurTrack = g_nTotalTracks = 0;
	g_pathVTrack = _T("");
	g_pathImageFile = _T("");
	g_lfStart = g_lfEnd = 0.0;
	g_hPlayStartedEvent = NULL;
	g_hEncodeStoppedEvent = NULL;

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void QMPInput::ShutDown(int flags)
{
	WCHAR inifile[MAX_PATH];

	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH*sizeof(TCHAR), 0);

	Stop( (char *)(LPCTSTR)g_pathVTrack, STOPFLAG_SHUTDOWN);

	// clean read cache
	g_listPlugins.RemoveAll();
}

//-----------------------------------------------------------------------------

int QMPInput::GetMediaSupported(const char* medianame, MediaInfo *mediaInfo)
{
	if ( !medianame || !*medianame)
		return FALSE;

	if ( QCUESheet::ParseVirtualTrackPath( (LPCTSTR)medianame)) {
		if ( PathFileExists( (LPCWSTR)medianame)) // virtual track, so return false for real path
			return FALSE;

		if ( mediaInfo) {
			mediaInfo->mediaType = DIGITAL_AUDIOFILE_MEDIA;
			mediaInfo->op_canSeek = TRUE;
		}
		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------

int QMPInput::GetTrackExtents(const char* medianame, TrackExtents *ext, int flags)
{
	CPath pathImageFile;
	int vtNum;
	QCUESheet cueSheet;

	if ( !cueSheet.ReadFromVirtualTrack( (LPCTSTR)medianame))
		return FALSE;

	vtNum = cueSheet.GetVirtualTrackNumber();
	pathImageFile = cueSheet.GetImageFilePath(vtNum);

	// create an input plug-in instance
	QCDModInitIn * cbs = _create_input_instance( pathImageFile);
	if ( !cbs)
		return FALSE;

	int ret = cbs->toModule.GetTrackExtents( _gen_module_path( cbs, pathImageFile), ext, flags);

	// release the input plug-in instance
	_release_input_instance( &cbs);

	if ( !ret)
		return FALSE;

	ext->track = vtNum; // we can always get the track number.
	ext->bytesize = 0; // always 0!
	ext->start = (UINT)(ext->unitpersec * cueSheet.GetTrackStartIndex( ext->track));
	if ( ext->start >= ext->end) // return FALSE when the start is not less than the end (the actual image file length)
		return FALSE;
	double total_seconds = ext->end * 1.0 / ext->unitpersec;
	ext->end = (UINT)(ext->unitpersec * cueSheet.GetTrackEndIndex( ext->track, total_seconds));

	return TRUE;
}

//-----------------------------------------------------------------------------

int QMPInput::Play(const char* medianame, int playfrom, int playto, int flags)
{
	int ret;

	switch (flags)
	{
	case PLAYFLAG_SEEKING:
		{
			ret = _play_on_seeking( medianame, playfrom, playto, flags);
		} break;

	case PLAYFLAG_ENCODING:
		{
			ret = _play_on_encoding( medianame, playfrom, playto, flags);
		} break;

	case PLAYFLAG_PLAYBACK:
		{
			ret = _play_on_playback( medianame, playfrom, playto, flags);
		} break;

	default:
		{
			ret = PLAYSTATUS_FAILED; // Oops! NEVER!!
		} break;
	}

	return ret;
}

//-----------------------------------------------------------------------------

int QMPInput::Stop(const char* medianame, int flags)
{
	if ( medianame && *medianame && !lstrcmpi( (LPCWSTR)medianame, g_pathVTrack)) {
		BOOL ret = TRUE;

		// Stop the playback.
		// DO NOT stop it again when the input plug-in has been shutdown!
		if ( 1 != QCDCallbacks.Service( opGetPlayerState, NULL, 0, 0)) {
			if ( g_hEncodeStoppedEvent) SetEvent( g_hEncodeStoppedEvent);

			ret = g_qcdCallbacks->toModule.Stop( _gen_module_path( g_qcdCallbacks, g_pathImageFile), flags);

			// unhook the player functions, redirect them back to original ones.
			_unhook_player_function( g_qcdCallbacks);

			// release the input plug-in instance
			_release_input_instance( &g_qcdCallbacks);
		}

		// reset all playback control variables
		g_nCurTrack = g_nTotalTracks = 0;
		g_pathVTrack = _T("");
		g_pathImageFile = _T("");
		g_lfStart = g_lfEnd = 0.0;

		if ( g_hPlayStartedEvent) {
			CloseHandle( g_hPlayStartedEvent);
			g_hPlayStartedEvent = NULL;
		}
		if ( g_hEncodeStoppedEvent) {
			CloseHandle( g_hEncodeStoppedEvent);
			g_hEncodeStoppedEvent = NULL;
		}

		return ret;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------

int QMPInput::Pause(const char* medianame, int flags)
{
	if ( 0 == lstrcmpi( (LPCWSTR)medianame, g_pathVTrack))
		return g_qcdCallbacks->toModule.Pause( _gen_module_path( g_qcdCallbacks, g_pathImageFile), flags);
	else
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
	g_qcdCallbacks->toModule.SetEQ( eqinfo);
}

//-----------------------------------------------------------------------------

void QMPInput::SetVolume(int levelleft, int levelright, int flags)
{
	if ( g_qcdCallbacks)
		g_qcdCallbacks->toModule.SetVolume( levelleft, levelright, flags);
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

	if ( g_qcdCallbacks)
		return g_qcdCallbacks->toModule.GetCurrentPosition( _gen_module_path( g_qcdCallbacks, (LPCTSTR)medianame), track, offset);
	else
		return 0;
}

//-----------------------------------------------------------------------------

void QMPInput::Configure(int flags)
{
}

//-----------------------------------------------------------------------------

void QMPInput::About(int flags)
{
	CAboutDlgInput dlg;
	dlg.DoModal( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0));
}

//-----------------------------------------------------------------------------

IQCDMediaDecoder* QMPInput::CreateDecoderInstance(const WCHAR* medianame, int flags)
{
	// TODO: return instance of class that implements the
	// IQCDMediaDecoder interface.

	return NULL;
}

//-----------------------------------------------------------------------------

QCDModInitIn * QMPInput::_create_input_instance(const CPath pathImageFile)
{
	if ( g_qcdCallbacks && !pathImageFile.GetExtension().CompareNoCase( g_pathImageFile.GetExtension()))
		return g_qcdCallbacks;

	// Let the player load the plug-in and check the support.
	// !! THIS IS THE BEST IMPORTANT STEP IN OUR CORE !!
	if ( UNKNOWN_MEDIA == QCDCallbacks.Service( opGetMediaSupported, (void *)(LPCTSTR)pathImageFile, 0, 0))
		return NULL;

	// Get the module handle of loaded plug-in
	HMODULE hmod = NULL;
	POSITION pos = g_listPlugins.GetHeadPosition();
	while ( pos != NULL) {
		const PLUGINCACHE & cache = g_listPlugins.GetNext( pos);
		if ( _is_supported( cache.exts, pathImageFile.GetExtension())) {
			// assume the player has loaded and initialized the plug-in.
			// we just return its handle directly.
			// !! THE SECOND IMPORTANT STEP IN OUR CORE !!
			// !! DO NOT MODIFY THE PLAYER'S CORE, JUST PEEK AND GRAB IT !!
			// !! DO NOT RELEASE THE HANDLE WHEN WE FINISHED OUR JOB !!
			hmod = GetModuleHandle( cache.path);
			if ( hmod) break;
		}
	}
	if ( !hmod)
		return NULL;

	// return the callbacks data of the plug-in
	pfnEntryPoint ep = (pfnEntryPoint)GetProcAddress( hmod, "QInputModule2");
	if ( !ep)
		return NULL;

	// invoke plug-in's entry point function
	QCDModInitIn * qcdcallbacks = ep();

	return qcdcallbacks;
}

void QMPInput::_release_input_instance(QCDModInitIn ** qcdcallbacks)
{
	if ( *qcdcallbacks)
		*qcdcallbacks = NULL;
}

void QMPInput::_hook_player_functions(QCDModInitIn * qcdcallbacks)
{
	//qcdcallbacks->Service = QCDCallbacks.Service;

	qcdcallbacks->toPlayer.PositionUpdate = PositionUpdate;
	qcdcallbacks->toPlayer.PlayStopped = PlayStopped;
	//qcdcallbacks->toPlayer.PlayStarted = PlayStarted;
	//qcdcallbacks->toPlayer.PlayPaused = PlayPaused;
	qcdcallbacks->toPlayer.PlayDone = PlayDone;
	//qcdcallbacks->toPlayer.PlayTrackChanged = PlayTrackChanged;
	//qcdcallbacks->toPlayer.MediaEjected = MediaEjected;
	//qcdcallbacks->toPlayer.MediaInserted = MediaInserted;
	qcdcallbacks->toPlayer.OutputOpen = OutputOpen;
	qcdcallbacks->toPlayer.OutputWrite = OutputWrite;
	//qcdcallbacks->toPlayer.OutputDrain = OutputDrain;
	//qcdcallbacks->toPlayer.OutputDrainCancel = OutputDrainCancel;
	//qcdcallbacks->toPlayer.OutputFlush = OutputFlush;
	qcdcallbacks->toPlayer.OutputStop = OutputStop;
	//qcdcallbacks->toPlayer.OutputPause = OutputPause;
	//qcdcallbacks->toPlayer.OutputSetVol = OutputSetVol;
	//qcdcallbacks->toPlayer.OutputGetCurrentPosition = OutputGetCurrentPosition;
	//qcdcallbacks->toPlayer.OutputTestFormat = OutputTestFormat;
}

void QMPInput::_unhook_player_function(QCDModInitIn * qcdcallbacks)
{
	//qcdcallbacks->Service = QCDCallbacks.Service;

	qcdcallbacks->toPlayer.PositionUpdate = QCDCallbacks.toPlayer.PositionUpdate;
	qcdcallbacks->toPlayer.PlayStopped = QCDCallbacks.toPlayer.PlayStopped;
	//qcdcallbacks->toPlayer.PlayStarted = QCDCallbacks.toPlayer.PlayStarted;
	//qcdcallbacks->toPlayer.PlayPaused = QCDCallbacks.toPlayer.PlayPaused;
	qcdcallbacks->toPlayer.PlayDone = QCDCallbacks.toPlayer.PlayDone;
	//qcdcallbacks->toPlayer.PlayTrackChanged = QCDCallbacks.toPlayer.PlayTrackChanged;
	//qcdcallbacks->toPlayer.MediaEjected = QCDCallbacks.toPlayer.MediaEjected;
	//qcdcallbacks->toPlayer.MediaInserted = QCDCallbacks.toPlayer.MediaInserted;
	qcdcallbacks->toPlayer.OutputOpen = QCDCallbacks.toPlayer.OutputOpen;
	qcdcallbacks->toPlayer.OutputWrite = QCDCallbacks.toPlayer.OutputWrite;
	//qcdcallbacks->toPlayer.OutputDrain = QCDCallbacks.toPlayer.OutputDrain;
	//qcdcallbacks->toPlayer.OutputDrainCancel = QCDCallbacks.toPlayer.OutputDrainCancel;
	//qcdcallbacks->toPlayer.OutputFlush = QCDCallbacks.toPlayer.OutputFlush;
	qcdcallbacks->toPlayer.OutputStop = QCDCallbacks.toPlayer.OutputStop;
	//qcdcallbacks->toPlayer.OutputPause = QCDCallbacks.toPlayer.OutputPause;
	//qcdcallbacks->toPlayer.OutputSetVol = QCDCallbacks.toPlayer.OutputSetVol;
	//qcdcallbacks->toPlayer.OutputGetCurrentPosition = QCDCallbacks.toPlayer.OutputGetCurrentPosition;
	//qcdcallbacks->toPlayer.OutputTestFormat = QCDCallbacks.toPlayer.OutputTestFormat;
}

/***********************************************************************
 ** Convert agent's UNICODE path string into module's path string
 **
 ** NOTE: the string format of our agent's path has be fixed in the
 **       entry pointer function.
 ***********************************************************************/
char * QMPInput::_gen_module_path(QCDModInitIn * cbs, LPCWSTR agentPath)
{
	char * p = NULL;

	if ( cbs->version == PLUGIN_API_VERSION_UNICODE) {
		p = (char *)(LPWSTR)(LPCWSTR)agentPath;
	} else if ( cbs->version == PLUGIN_API_VERSION_NTUTF8) {
		UCS2toUTF8(agentPath, g_strModulePath);
		p = (char *)(LPSTR)(LPCSTR)g_strModulePath;
	}
	else if ( cbs->version == PLUGIN_API_VERSION) {
		UCS2toMB(agentPath, g_strModulePath);
		p = (char *)(LPSTR)(LPCSTR)g_strModulePath;
	} else {
		p = NULL;
	}

	return p;
}

/***********************************************************************
 ** Convert module's path string into agent's UNICODE path string
 **
 ** NOTE: the string format of our agent's path has be fixed in the
 **       entry pointer function.
 ***********************************************************************/
LPWSTR QMPInput::_gen_agent_path(QCDModInitIn * cbs, const char * modulePath)
{
	LPWSTR p = NULL;

	if ( cbs->version == PLUGIN_API_VERSION_UNICODE) {
		p = (LPWSTR)(LPCWSTR)modulePath;
	} else if ( cbs->version == PLUGIN_API_VERSION_NTUTF8) {
		UTF8toUCS2(modulePath, g_wstrAgentPath);
		p = (LPWSTR)(LPCWSTR)g_wstrAgentPath;
	} else if ( cbs->version == PLUGIN_API_VERSION) {
		MBtoUCS2(modulePath, g_wstrAgentPath);
		p = (LPWSTR)(LPCWSTR)g_wstrAgentPath;
	} else {
		p = NULL;
	}

	return p;
}

int QMPInput::_play_on_seeking(const char* medianame, int playfrom, int playto, int flags)
{
	return g_qcdCallbacks->toModule.Play( _gen_module_path( g_qcdCallbacks, g_pathImageFile), playfrom, g_tePlaying.end, flags);
}

int QMPInput::_play_on_playback(const char* medianame, int playfrom, int playto, int flags)
{
	// parse virtual track
	QCUESheet cueSheet;
	if ( !cueSheet.ReadFromVirtualTrack( (LPCTSTR)medianame)) return FALSE;
	int vtNum = cueSheet.GetVirtualTrackNumber();
	CPath pathImageFile = cueSheet.GetImageFilePath(vtNum);

	if ( lstrcmpi( pathImageFile, g_pathImageFile)) { // a different/new image file -- init input plugin
		// close previous track
		if ( lstrlen( g_pathImageFile) > 0) {
			SetEvent( g_hPlayStartedEvent);

			QCDCallbacks.toPlayer.OutputStop( STOPFLAG_PLAYDONE);
			Stop( (const char *)(LPCTSTR)g_pathVTrack, STOPFLAG_PLAYDONE);
		}

		// release the existent instance
		_release_input_instance( &g_qcdCallbacks);

		// Load input plug-in, but NOT initializes it. QMP will do this.
		g_qcdCallbacks = _create_input_instance( pathImageFile);
		if ( !g_qcdCallbacks)
			return PLAYSTATUS_UNSUPPORTED; // no supported plug-ins for decoding, so, return UNSUPPORTED to player.

		// get the total length of the image file and send it to plug-in as "playto"
		if ( !g_qcdCallbacks->toModule.GetTrackExtents( _gen_module_path( g_qcdCallbacks, pathImageFile), &g_tePlaying, 0))
			return PLAYSTATUS_FAILED;

		// hook the player functions and redirect them to our customized functions
		_hook_player_functions( g_qcdCallbacks);

		// save total tracks count of the image file from the cue sheet
		g_nTotalTracks = cueSheet.GetNumTracks();

		g_hPlayStartedEvent = CreateEvent( NULL, TRUE, TRUE, NULL); // signaled state

		// save playback info.
		g_lfStart = playfrom;
		g_lfEnd = playto;
		g_nCurTrack = vtNum;
		g_pathImageFile = pathImageFile; // save new image file
		g_pathVTrack = (LPCTSTR)medianame;

		// do playback
		return g_qcdCallbacks->toModule.Play( _gen_module_path( g_qcdCallbacks, g_pathImageFile), playfrom, g_tePlaying.end, flags);
	} else { // a different virtual track in the same image file -- actually, seeking
		g_lfStart = playfrom;
		g_lfEnd = playto;
		g_pathVTrack = (LPCTSTR)medianame;

		// make sure we always in the signaled state
		SetEvent( g_hPlayStartedEvent);

		if ( (g_nCurTrack + 1) == vtNum) { // Seamless Playback mode
			g_nCurTrack = vtNum;
			return PLAYSTATUS_SUCCESS; // return immediately when virtual track play done.
		} else {
			g_nCurTrack = vtNum;
			return g_qcdCallbacks->toModule.Play( _gen_module_path( g_qcdCallbacks, g_pathImageFile), playfrom, g_tePlaying.end, flags);
		}
	}
}

int QMPInput::_play_on_encoding(const char* medianame, int playfrom, int playto, int flags)
{
	// parse virtual track
	QCUESheet cueSheet;
	if ( !cueSheet.ReadFromVirtualTrack( (LPCTSTR)medianame)) return FALSE;
	int vtNum = cueSheet.GetVirtualTrackNumber();
	CPath pathImageFile = cueSheet.GetImageFilePath(vtNum);

	/** encoding always open a new track file **/

	// release the existent instance
	_release_input_instance( &g_qcdCallbacks);

	// Load input plug-in, but NOT initializes it. QMP will do this.
	g_qcdCallbacks = _create_input_instance( pathImageFile);
	if ( !g_qcdCallbacks)
		return PLAYSTATUS_UNSUPPORTED; // no supported plug-ins for decoding, so, return UNSUPPORTED to player.

	// get the total length of the image file and send it to plug-in as "playto"
	if ( !g_qcdCallbacks->toModule.GetTrackExtents( _gen_module_path( g_qcdCallbacks, pathImageFile), &g_tePlaying, 0))
		return PLAYSTATUS_FAILED;

	// hook the player functions and redirect them to our customized functions
	_hook_player_functions( g_qcdCallbacks);

	// save total tracks count of the image file from the cue sheet
	g_nTotalTracks = cueSheet.GetNumTracks();

	g_hEncodeStoppedEvent = CreateEvent( NULL, TRUE, FALSE, NULL); // nonsignaled state

	g_lfStart = playfrom;
	g_lfEnd = playto;

	g_nCurTrack = vtNum;
	g_pathImageFile = pathImageFile;
	g_pathVTrack = (LPCTSTR)medianame;

	return g_qcdCallbacks->toModule.Play( _gen_module_path( g_qcdCallbacks, g_pathImageFile), playfrom, g_tePlaying.end, flags);
}

