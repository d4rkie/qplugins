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

#include <list>
using namespace std;

#include "QCUESheet.h"

#include "AboutDlg.h"

//-----------------------------------------------------------------------------

WCHAR g_szInputModDisplayStr[1024] = {0};

HWND			QMPInput::hwndPlayer;
QCDModInitIn	QMPInput::QCDCallbacks;

typedef struct __PLUGINCACHE {
	int number;   // loading number
	CPath path;     // full path of plugin
	CString exts; // supported extensions in form ":.ext1:.ext2:^^^:.extN:"
} PLUGINCACHE;
list<PLUGINCACHE> g_listPlugins;

typedef QCDModInitIn * (*pfnEntryPoint)();

// for playing architecture
int g_nCurTrack; // current track number
int g_nTotalTracks; // total track counts of current image file
QCDModInitIn * g_qcdCallbacks; // our callback for decoding
CPath g_pathVTrack; // current virtual track file name
CPath g_pathImageFile; // playing image file name
double g_lfStart, g_lfEnd; // the position for playback
TrackExtents g_tePlaying; // TrackExtents of the playing image file

BOOL g_bChanged;

HANDLE g_hSignal;

CStringA g_strModulePath; // store Multi-Bytes/UTF-8 string
CStringW g_wstrAgentPath; // store UNICODE string

//-----------------------------------------------------------------------------

/************************************************************************
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

/************************************************************************
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

/************************************************************************
 ** Hook the data output service
 **
 **
 ***********************************************************************/
int QMPInput::OutputWrite(WriteDataStruct * wd)
{
	if ( wd->markerstart >= g_lfEnd && g_nCurTrack < g_nTotalTracks) { // ready to switch?
		double cur = QCDCallbacks.Service( opGetOutputTime, NULL, 0, 0) / 1000.0 * g_tePlaying.unitpersec;
		if ( cur >= (g_lfEnd - g_lfStart)) { // It's time to switch!
			g_bChanged = TRUE;
			ResetEvent( g_hSignal);

			QCDCallbacks.toPlayer.PlayDone( (char *)(LPCTSTR)g_pathVTrack, 0);
			WaitForSingleObject( g_hSignal, INFINITE); // Wait until the new g_lfStart is set
		}
	}

	if ( (wd->markerstart * 1.0) > g_lfStart)
		wd->markerstart = (unsigned int)(wd->markerstart * 1.0 - g_lfStart);

	return QCDCallbacks.toPlayer.OutputWrite( wd);
}

/************************************************************************
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

/************************************************************************
 ** Hook the Play Stop service
 **
 ** When plug-in invoke "PlayStopped", stop for virtual track
 ***********************************************************************/
void QMPInput::PlayStopped(const char* medianame, int flags)
{
	QCDCallbacks.toPlayer.PlayStopped( (const char *)(LPCTSTR)g_pathVTrack, flags);
}

/************************************************************************
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

bool _my_compare(const PLUGINCACHE & pc1, const PLUGINCACHE & pc2)
{
	return pc1.number < pc2.number;
}

/************************************************************************
 ** Read all input plug-ins from cache file
 **
 ***********************************************************************/
void _read_plugin_cache(const CPath & pathPluginFolder)
{
	TCHAR buf[1000];
	TCHAR myfn[MAX_PATH];
	LPTSTR p;
	CPath inifile;

	inifile = pathPluginFolder; inifile.Append( _T("PluginCache.ini"));
	if ( !inifile.FileExists())
		return;

	// get filename of myself
	GetModuleFileName( g_hInstance, myfn, MAX_PATH);
	PathStripPath( myfn);

	// get all cached plug-in filename
	ZeroMemory( buf, 1000);
	GetPrivateProfileSectionNames( buf, 1000, inifile);
	if ( buf[0] == _T('\0')) // no section cached
		return;

	// start processing
	p = buf;
	while ( *p) {
		// concatenate a full path of plug-in file
		CPath pathPluginFile = pathPluginFolder; pathPluginFile.Append( p);

		// process all existent plug-ins except myself
		if ( pathPluginFile.FileExists() && lstrcmpi( p, myfn)) {
			TCHAR value[200];
			GetPrivateProfileString( p, _T("Input0"), _T(""), value, 10, inifile);
			if ( _tcsnicmp( value, _T("TRUE"), 4) == 0) {
				int num;
				CString exts;

				GetPrivateProfileString( p, _T("Exts20"), _T(""), value, 200, inifile);

				num = GetPrivateProfileInt( p, _T("Param20"), 0, inifile);

				// make the supported ext in the form of ":.ext1:.ext2:...:.extN:"
				exts = value; exts.Replace( _T(":"), _T(":."));
				exts = _T(":.") + exts + _T(":");

				// push input plug-in info.
				PLUGINCACHE pc = { num, pathPluginFile, exts.MakeUpper() };
				g_listPlugins.push_back( pc);
			}
		}

		// next section name(plug-in name)
		p += (lstrlen(p)+1);
	}
}

//-----------------------------------------------------------------------------

int QMPInput::Initialize(QCDModInfo *ModInfo, int flags)
{
	WCHAR inifile[MAX_PATH], pluginfldr[MAX_PATH];
	int i;

	ModInfo->moduleString = (LPSTR)g_szInputModDisplayStr;
	ModInfo->moduleExtensions = (LPSTR)L"VT"; // virtual track

	// load display name
	ResInfo resInfo = { sizeof(ResInfo), g_hInstance, MAKEINTRESOURCE(IDS_INPUT_MODULE), 0, 0 };
	QCDCallbacks.Service( opLoadResString, (void*)g_szInputModDisplayStr, (long)sizeof(g_szInputModDisplayStr), (long)&resInfo);
#ifdef _DEBUG
	lstrcat( g_szInputModDisplayStr, _T("(debug)"));
#endif


	hwndPlayer = (HWND)QCDCallbacks.Service(opGetParentWnd, 0, 0, 0);

	// load player's input plug-in which will be used by ourself
	QCDCallbacks.Service( opGetPlayerSettingsFile, inifile, MAX_PATH, 0);
	g_listPlugins.clear();
	i = 0;
	do {
		TCHAR buf[20];
		wsprintf( buf, _T("PluginFolder%i"), i++);
		GetPrivateProfileString( _T("Folders"), buf, _T(""), pluginfldr, MAX_PATH, inifile);
		if ( lstrlen( pluginfldr))
			_read_plugin_cache( pluginfldr);
		else
			break;
	} while ( true);
	g_listPlugins.sort( _my_compare);

	// init
	g_nCurTrack = g_nTotalTracks = 0;
	g_pathVTrack = _T("");
	g_pathImageFile = _T("");
	g_lfStart = g_lfEnd = 0.0;
	g_bChanged = FALSE;
	g_hSignal = NULL;

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void QMPInput::ShutDown(int flags)
{
	WCHAR inifile[MAX_PATH];

	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	Stop( (char *)(LPCTSTR)g_pathVTrack, STOPFLAG_SHUTDOWN);
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

#ifdef _DEBUG
	QLogFile::GetInst().OutputLogStr( _T("input@GetTrackExtents")
	                                 , _T("Image File: track: %d, start: %d, end: %d, unitpersec: %d, bytesize: %d")
	                                 , ext->track, ext->start, ext->end, ext->unitpersec, ext->bytesize
	                                 );
#endif

	ext->track = vtNum; // we can always get the track number.
	ext->bytesize = 0; // always 0!
	ext->start = (UINT)(cueSheet.GetTrackStartIndex( ext->track) * ext->unitpersec);
	if ( ext->start >= ext->end) // return FALSE when the start is not less than the end (the actual image file length)
		return FALSE;
	double total_seconds = ext->end * 1.0 / ext->unitpersec;
	ext->end = (UINT)(cueSheet.GetTrackEndIndex( ext->track, total_seconds) * ext->unitpersec);

#ifdef _DEBUG
	QLogFile::GetInst().OutputLogStr( _T("input@GetTrackExtents")
	                                 , _T("Virtual Track: track: %d, start: %d, end: %d, unitpersec: %d, bytesize: %d")
	                                 , ext->track, ext->start, ext->end, ext->unitpersec, ext->bytesize
	                                 );
#endif

	return TRUE;
}

//-----------------------------------------------------------------------------

int QMPInput::Play(const char* medianame, int playfrom, int playto, int flags)
{
	int ret;

	if ( flags != PLAYFLAG_SEEKING) { // for playback/encoding
		// parse virtual track
		CPath pathImageFile;
		int vtNum;
		QCUESheet cueSheet;

		if ( !cueSheet.ReadFromVirtualTrack( (LPCTSTR)medianame))
			return FALSE;

		vtNum = cueSheet.GetVirtualTrackNumber();
		pathImageFile = cueSheet.GetImageFilePath(vtNum);

		// a different/new image file -- init input plugin
		if ( lstrcmpi( pathImageFile, g_pathImageFile)) {
			if ( lstrlen( g_pathImageFile) > 0) { // close previous track
				// fix for playdone
				if ( g_bChanged) {
					g_bChanged = FALSE;
					SetEvent( g_hSignal);
				}
				QCDCallbacks.toPlayer.OutputStop( STOPFLAG_PLAYDONE);
				Stop( (const char *)(LPCTSTR)g_pathVTrack, STOPFLAG_PLAYDONE);
			}

			// release the existent instance
			_release_input_instance( &g_qcdCallbacks);

#ifdef _DEBUG
			QLogFile::GetInst().OutputLogStr( _T("input@Play"), _T("loading plugin..."));
#endif

			// Load input plug-in, but NOT initializes it. QMP will do this.
			g_qcdCallbacks = _create_input_instance( pathImageFile);
			if ( !g_qcdCallbacks)
				return PLAYSTATUS_UNSUPPORTED; // no supported plug-ins for decoding, so, return UNSUPPORTED to player.

#ifdef _DEBUG
			QLogFile::GetInst().OutputLogStr( _T("input@Play"), _T("initializing plugin..."));
#endif

			// get the total length of the image file and send it to plug-in as "playto"
			if ( !g_qcdCallbacks->toModule.GetTrackExtents( _gen_module_path( g_qcdCallbacks, pathImageFile), &g_tePlaying, 0))
				return PLAYSTATUS_FAILED;

			// hook the player functions and redirect them to our customized functions
			_hook_player_functions( g_qcdCallbacks);

			// save total tracks count of the image file from the cue sheet
			g_nTotalTracks = cueSheet.GetNumTracks();

			g_hSignal = CreateEvent( NULL, TRUE, TRUE, NULL);
		} else {
			// Seeking in the same image file
			flags |= PLAYFLAG_SEEKING;
		}

		g_lfStart = playfrom;
		g_lfEnd = playto;

		g_nCurTrack = vtNum;
		g_pathImageFile = pathImageFile;
		g_pathVTrack = (LPCTSTR)medianame;

#ifdef _DEBUG
		QLogFile::GetInst().OutputLogStr( _T("input@Play"), _T("Start playing/encoding: from %d, to %d"), playfrom, g_tePlaying.end);
#endif

		if ( PLAYFLAG_ENCODING == flags) {
			// Encoding mode: play new track, then notify playdone
			ret = g_qcdCallbacks->toModule.Play( _gen_module_path( g_qcdCallbacks, g_pathImageFile), playfrom, g_tePlaying.end, flags);

			// fix for playdone
			if ( g_bChanged) {
				g_bChanged = FALSE;
				SetEvent( g_hSignal);
			}
		} else {
			// Seamless Playback mode: notify playdone, then play new track
			// fix for playdone
			if ( g_bChanged) {
				g_bChanged = FALSE;
				SetEvent( g_hSignal);
				return PLAYSTATUS_SUCCESS; // return immediately when virtual track play done.
			}

			ret = g_qcdCallbacks->toModule.Play( _gen_module_path( g_qcdCallbacks, g_pathImageFile), playfrom, g_tePlaying.end, flags);
		}

		return ret;
	} else {
		return g_qcdCallbacks->toModule.Play( _gen_module_path( g_qcdCallbacks, g_pathImageFile), playfrom, g_tePlaying.end, flags);
	}

	// invoke input plug-in's "Play" to play it.
	// NOTE: always set playto to the duration of image file
	// for Seeking: return immediately
}

//-----------------------------------------------------------------------------

int QMPInput::Stop(const char* medianame, int flags)
{
	if ( medianame && *medianame && !lstrcmpi( (LPCWSTR)medianame, g_pathVTrack)) {
		BOOL ret = TRUE;

		// stop the playback 
		// DO NOT stop it again when the input plug-in has been shutdown!
		if ( 1 != QCDCallbacks.Service( opGetPlayerState, NULL, 0, 0)) {
			ret = g_qcdCallbacks->toModule.Stop( _gen_module_path( g_qcdCallbacks, g_pathImageFile), flags);

			// unhook the player functions, redirect them back to original ones.
			_unhook_player_function( g_qcdCallbacks);

			// release the input plug-in instance
			_release_input_instance( &g_qcdCallbacks);
		}

		// reset all playback control vars
		g_nCurTrack = g_nTotalTracks = 0;
		g_pathVTrack = _T("");
		g_pathImageFile = _T("");
		g_lfStart = g_lfEnd = 0.0;
		g_bChanged = FALSE;

		CloseHandle( g_hSignal);
		g_hSignal = NULL;

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
	// !! This is the best important step of our core. !!
	if ( UNKNOWN_MEDIA == QCDCallbacks.Service( opGetMediaSupported, (void *)(LPCTSTR)pathImageFile, 0, 0))
		return NULL;

	// Get the module handle of loaded plug-in
	HMODULE hmod = NULL;
	for ( list<PLUGINCACHE>::iterator it = g_listPlugins.begin(); it != g_listPlugins.end(); it++) {
		if ( _is_supported( (*it).exts, pathImageFile.GetExtension())) {
			// assume the player has loaded and initialized the plug-in.
			// we just return its handle directly.
			hmod = GetModuleHandle( (*it).path);

#ifdef _DEBUG
			if ( hmod) QLogFile::GetInst().OutputLogStr( _T("input"), _T("%d: %s has been loaded!"), hmod, (*it).path);
#endif

			break;
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

#ifdef _DEBUG
	QLogFile::GetInst().OutputLogStr( _T("input"), _T("initialize %d %s!"), hmod, qcdcallbacks ? "successful" : "unsuccessfully");
#endif

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

/************************************************************************
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

/************************************************************************
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

