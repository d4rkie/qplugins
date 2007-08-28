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
#include "QMPTAK.h"
#include "QMPInput.h"

#include "QDecoder.h"

#include "AboutDlg.h"


//////////////////////////////////////////////////////////////////////////
// Static Class member variables
HWND			QMPInput::hwndPlayer;
QCDModInitIn	QMPInput::QCDCallbacks;

//////////////////////////////////////////////////////////////////////////

// plugin name and supported extensions
//     must be global since player has reference to these
WCHAR g_szPluginDisplayStr[1024] = {0};
CStringW g_strPluginExtensions;

HANDLE	g_hDecoderThread = NULL;
BOOL	g_bDecoderThreadKill = FALSE;

typedef struct {
	QMediaReader mediaReader;
	DWORD playFrom;
} DecodeThreadData_t;
DecodeThreadData_t * g_pThreadData = NULL;

// playback states
BOOL	g_bIsPaused = FALSE;
BOOL	g_bSeekNeeded = FALSE;
DWORD	g_dwSeekPos = 0;
CStringW g_strCurrentMedia;

//-----------------------------------------------------------------------------

int QMPInput::Initialize(QCDModInfo *ModInfo, int flags)
{
	// module info for plugin remains pointed to these strings
	ModInfo->moduleString = (char*)g_szPluginDisplayStr;
	ModInfo->moduleExtensions = (char*)L"TAK";//(char*)(LPCWSTR)g_strPluginExtensions;

	// where am i? 
	WCHAR modulePath[MAX_PATH] = {0};
	GetModuleFileName( g_hInstance, modulePath, MAX_PATH);

	WCHAR pluginFolder[MAX_PATH] = {0};
	QCDCallbacks.Service( opGetPluginFolder, (void*)pluginFolder, sizeof(pluginFolder), (long)modulePath);

	// load tak engine dll (was set to be delay loaded)
	WCHAR takPlugin[MAX_PATH+20] = {0}; lstrcpy( takPlugin, pluginFolder);
	PathAppend( takPlugin, L"tak_deco_lib.dll");

	HINSTANCE hInst = LoadLibrary( takPlugin);
	if ( !hInst) hInst = LoadLibrary( L"tak_deco_lib.dll");
	if ( !hInst) {
		// Could not find TAK engine
		// return TRUE, so plugin stays loaded, but remove plugin calls so 
		// player cant utilize. Also change plugin display string.

		memset(&QCDCallbacks.toModule, 0, sizeof(QCDCallbacks.toModule));
		QCDCallbacks.toModule.Initialize	= Initialize;
		QCDCallbacks.toModule.ShutDown		= ShutDown;

		ResInfo resInfo = { sizeof(ResInfo), g_hInstance, MAKEINTRESOURCE(IDS_DISPLAYNAME_NOTFOUND), 0, 0 };
		QCDCallbacks.Service(opLoadResString, (void*)g_szPluginDisplayStr, (long)sizeof(g_szPluginDisplayStr), (long)&resInfo);
		return TRUE;
	}

	// load display name
	ResInfo resInfo = { sizeof(ResInfo), g_hInstance, MAKEINTRESOURCE(IDS_DISPLAYNAME), 0, 0 };
	QCDCallbacks.Service(opLoadResString, (void*)g_szPluginDisplayStr, (long)sizeof(g_szPluginDisplayStr), (long)&resInfo);

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void QMPInput::ShutDown(int flags)
{
	Stop( NULL, STOPFLAG_FORCESTOP);
	//SaveSettings();
}

//-----------------------------------------------------------------------------

int QMPInput::GetMediaSupported(const char* medianame, MediaInfo *mediaInfo)
{
	if ( !medianame || !*medianame)
		return FALSE;

	// blindly accept streams?
	if ( PathIsURLW( (LPCWSTR)medianame)) {
		if ( mediaInfo) {
			mediaInfo->mediaType = DIGITAL_AUDIOSTREAM_MEDIA;
		}
		return TRUE;
	}

	WCHAR *testExt = (WCHAR*)wcsrchr((LPCWSTR)medianame, '.');
	if (!testExt)
		return FALSE;
	testExt++;

	QDecoder dec;
	if ( dec.IsOurFile( (LPCWSTR)medianame)) {
		if (mediaInfo) {
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
	if ( PathIsURLW( (LPCWSTR)medianame)) {
		ext->track = 1;
		ext->bytesize = 0;
		ext->start = 0;
		ext->end = 1;
		ext->unitpersec = 1000;
		return TRUE;
	}

	BOOL ret = FALSE;
	QDecoder dec;
	QMediaReader mr((IQCDMediaSource *)QCDCallbacks.Service( opGetIQCDMediaSource, (void*)medianame, 0, 0));

	if ( !mr.Open()) return FALSE;
	ret = (NOERROR == dec.GetTrackExtents( mr, *ext));
	mr.Close();

	return ret;
}

//-----------------------------------------------------------------------------

int QMPInput::Play(const char* medianame, int playfrom, int playto, int flags)
{
	int ret = FALSE;

	// a new file is being sent for play, stop whatever we were doing
	if ( !g_strCurrentMedia.IsEmpty() && g_strCurrentMedia.CompareNoCase( (LPCWSTR)medianame) != 0)
		Stop( NULL, STOPFLAG_PLAYSKIP);

	if ( g_hDecoderThread) { // playback thread active
		if ( g_bIsPaused) { // playback is paused, unpause on play
			// Update the player controls to reflect the new unpaused state
			QCDCallbacks.toPlayer.OutputPause(0);

			Pause( medianame, 0);

			if ( playfrom >= 0) {
				g_dwSeekPos = (DWORD)playfrom;

				g_bSeekNeeded = TRUE;
				if ( g_pThreadData) g_pThreadData->mediaReader.Reset();
			}

			ret = TRUE;
		} else { // is playing, do seek
			if ( playfrom >= 0) {
				g_dwSeekPos = (DWORD)playfrom;
				g_bSeekNeeded = TRUE;
				if ( g_pThreadData) g_pThreadData->mediaReader.Reset();
			}

			ret = TRUE;
		}
	} else { // not currently playing, start play
		// create decoding thread
		g_pThreadData = new DecodeThreadData_t;
		if ( g_pThreadData) {
			g_pThreadData->playFrom = playfrom; // start of playback
			IQCDMediaSource * pms = (IQCDMediaSource *)QCDCallbacks.Service( opGetIQCDMediaSource, (void*)medianame, 0, 0);
			if ( pms) {
				UINT dwTId;

				// attach mediasource interface to mediareader wrapper with a reading loop killing signal
				g_pThreadData->mediaReader.Attach( pms);

				g_bDecoderThreadKill = FALSE;
				g_hDecoderThread = (HANDLE)_beginthreadex(NULL, 0, DecodeThread, (void*)(&g_pThreadData), CREATE_SUSPENDED, &dwTId);
				if ( g_hDecoderThread) {
					// like to run the decoder with good responsiveness
					SetThreadPriority( g_hDecoderThread, THREAD_PRIORITY_HIGHEST);
					ret = TRUE;

					g_strCurrentMedia = (LPCWSTR)medianame;

					ResumeThread( g_hDecoderThread);
				}
			}

			if ( !g_hDecoderThread) {
				delete g_pThreadData;
				g_pThreadData = NULL;
			}
		}
	}

	return ret;
}

//-----------------------------------------------------------------------------

int QMPInput::Stop(const char* medianame, int flags)
{
	// I'm choosing to just ignore the medianame and
	// trust we're always talking about whatever is
	// currently playing

	QCDCallbacks.toPlayer.OutputStop(flags);

	if ( g_hDecoderThread) {
		if ( g_pThreadData) g_pThreadData->mediaReader.Reset();
		g_bDecoderThreadKill = TRUE;

		WaitForSingleObject( g_hDecoderThread, INFINITE);
		if ( g_hDecoderThread) {
			CloseHandle( g_hDecoderThread);
			g_hDecoderThread = NULL;
		}

		assert(g_hDecoderThread == NULL);
	}

	g_strCurrentMedia.Empty();
	g_bIsPaused = 0;

	return TRUE;
}

//-----------------------------------------------------------------------------

int QMPInput::Pause(const char* medianame, int flags)
{
	g_bIsPaused = flags;

	if ( QCDCallbacks.toPlayer.OutputPause(flags)) {
		// send back pause/unpause notification
		return TRUE;
	}

	g_bIsPaused = !flags;
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
	// Not implemented yet!
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
	if ( !medianame || !*medianame)
		return NULL;

	if ( PathIsURLW( medianame))
		return NULL; // no streams for now

	QDecoder * pDec = new QDecoder;
	QMediaReader * pmr = new QMediaReader((IQCDMediaSource *)QCDCallbacks.Service( opGetIQCDMediaSource, (void*)medianame, 0, 0));

	if ( !pDec || !pmr)
		return NULL;

	return pDec->CreateDecoderInstance( pmr, flags);
}

//-----------------------------------------------------------------------------
// IQCDMediaSourceStatus impl

class MediaSourceStatus : public IQCDMediaSourceStatus
{
	QMediaReader * m_pMediaReader;
	PluginServiceFunc m_opService;

	void _metadataAvailable()
	{
		IQCDMediaInfo* pMediaInfo = (IQCDMediaInfo*)m_opService( opGetIQCDMediaInfo, 0, 0, 0);
		if ( pMediaInfo) {
			if ( m_pMediaReader->GetAvailableMetadata( pMediaInfo) > 0) {
				IQCDMediaInfo *pPlayerInfo = (IQCDMediaInfo*)m_opService( opGetIQCDMediaInfo, (void*)(LPCWSTR)m_pMediaReader->GetName(), DIGITAL_AUDIOSTREAM_MEDIA, 0);
				if ( pPlayerInfo) {
					WCHAR szName[1024], szValue[1024], szURL[1024] = {0};
					long nameLen = 1024, valueLen = 1024, index = 0;

					while ( pMediaInfo->GetInfoByIndex( index, szName, &nameLen, szValue, &valueLen)) {
						if ( lstrcmpiW( szName, L"StreamTitle") == 0) {
							pPlayerInfo->SetInfoByName( QCDInfo_TitleTrack, szValue, 0);
						} else if ( (lstrcmpiW( szName, L"icy-name") == 0) ||
						            (lstrcmpiW( szName, L"ice-name") == 0)
						          ) {
							pPlayerInfo->SetInfoByName( QCDInfo_ArtistAlbum, szValue, 0);
						} else if ( lstrcmpiW( szName, L"StreamUrl") == 0) {
							if ( wcsstr( szValue, L"://"))
								lstrcpynW( szURL, szValue, 1024);
						} else if ( (lstrcmpiW( szName, L"icy-url") == 0) ||
						            (lstrcmpiW( szName, L"ice-url") == 0)
						          ) {
							if ( (szURL[0] == 0) && wcsstr( szValue, L"://"))
								lstrcpynW( szURL, szValue, 1024);
						}

						index++;
						nameLen = 1024;
						valueLen = 1024;
					}

					// TODO: need to test that metadata changed before applying
					// (saves all the effort)

					pPlayerInfo->ApplyToAll(0);
					pPlayerInfo->Release();

					if ( szURL[0]) m_opService( opSetBrowserUrl, (void*)szURL, 0, 0);
				}
			}

			pMediaInfo->Release();
		}
	};

public:
	MediaSourceStatus(QMediaReader & mediaReader, PluginServiceFunc opService)
	{
		m_pMediaReader = &mediaReader;
		m_opService = opService;
		m_pMediaReader->SetStatusCallback( this, 0);
	};

	void __stdcall StatusMessage(long statusFlag, LPCWSTR statusMsg, long userData)
	{
		if ( statusFlag == MEDIASOURCE_STATUS_RECEIVEDMETADATA)
			_metadataAvailable();
		else if ( statusMsg && *statusMsg)
			m_opService( opSetStatusMessage, (void*)statusMsg, TEXT_TOOLTIP|TEXT_UNICODE, 0);
	};
};

//-----------------------------------------------------------------------------

UINT WINAPI __stdcall QMPInput::DecodeThread(void * b)
{
	DecodeThreadData_t ** ppthreadData = (DecodeThreadData_t **)b;
	DecodeThreadData_t * threadData = *ppthreadData;
	assert(threadData);

	bool bOutputOpened = false;

	MediaSourceStatus* pReaderStatus = new MediaSourceStatus(threadData->mediaReader, QCDCallbacks.Service);
	QDecoder * pQDecoder = NULL;
	WAVEFORMATEX wfex;
	bool bSuccess = false;
	do {
		if ( !threadData->mediaReader.Open())
			break;

		pQDecoder = new QDecoder;
		if ( !pQDecoder)
			break;

		if ( !pQDecoder->Open( threadData->mediaReader))
			break;

		bSuccess = true;
	} while (0);

	// no joy, tell player, kill decoder
	if ( !bSuccess) {
		//WCHAR title[300] = {0}, msg[300] = {0};

		//ResInfo resInfo = { sizeof(ResInfo), g_hInstance, MAKEINTRESOURCE(IDS_ERR_TITLE), 0, 0 };
		//QCDCallbacks.Service(opLoadResString, (void*)title, (long)sizeof(title), (long)&resInfo);

		//resInfo.resID = MAKEINTRESOURCE(IDS_ERR_BADFILE);
		//QCDCallbacks.Service(opLoadResString, (void*)msg, (long)sizeof(msg), (long)&resInfo);

		//HWND hwndPlayer = (HWND)QCDCallbacks.Service(opGetParentWnd, 0, 0, 0);
		//MessageBox(hwndPlayer, msg, title, MB_ICONINFORMATION);

		QCDCallbacks.toPlayer.PlayStopped( (LPCSTR)(LPCWSTR)threadData->mediaReader.GetName(), 0);
		g_bDecoderThreadKill = TRUE;
		threadData->mediaReader.Reset();
	} else {
		// Is media seekable?
		QCDCallbacks.Service( opSetTrackSeekable, (void*)(LPCWSTR)threadData->mediaReader.GetName(), pQDecoder->IsSeekable(), 0);
	}

	AudioInfo aiLast = {sizeof(AudioInfo), 0, 0, 0, 0, 0, ""};
	BOOL bDecodeDone = FALSE;

	while ( !g_bDecoderThreadKill && !bDecodeDone) {
		//
		// Perform Seeking
		//
		if ( !bDecodeDone && g_bSeekNeeded) {
			QCDCallbacks.toPlayer.OutputFlush(0);
			g_bSeekNeeded = FALSE;

			pQDecoder->Seek( g_dwSeekPos);
		}

		//
		// Perform Decoding
		//
		if ( !pQDecoder->IsActive()) {
			bDecodeDone = TRUE;
		} else {
			WriteDataStruct wd; ZeroMemory( &wd, sizeof(WriteDataStruct));

			if ( pQDecoder->Decode( wd)) {
				// Should we seek first?
				if ( threadData->playFrom > 0) {
					if ( !pQDecoder->Seek( threadData->playFrom))
						break;

					threadData->playFrom = 0;

					continue; // decoding from new position
				}

				//
				// open output after decoding to real start of playback
				//
				if ( !bOutputOpened) {
					pQDecoder->GetWaveFormFormat( wfex);
					if ( !QCDCallbacks.toPlayer.OutputOpen( (LPCSTR)(LPCWSTR)threadData->mediaReader.GetName(), &wfex))
						break;

					bOutputOpened = true;
				}

				//
				// setup AudioInfo
				//
				AudioInfo ai;
				memset(&ai, 0, sizeof(ai));
				ai.struct_size = sizeof(AudioInfo);
				pQDecoder->GetAudioInfo( ai);
				if ( memcmp( &ai, &aiLast, sizeof(AudioInfo))) { // change audio info when necessary
					if ( !(threadData->mediaReader.GetProperties() & MEDIASOURCE_PROP_INTERNETSTREAM)) {
						int ch = threadData->mediaReader.GetName().ReverseFind( _T('.'));
						if ( ch > 0) {
							ZeroMemory( ai.text, sizeof(ai.text));
							WideCharToMultiByte( CP_ACP, 0, threadData->mediaReader.GetName().Mid(ch+1), -1, ai.text, sizeof(ai.text), 0, 0);
							CharUpperA(ai.text);
						}
					}
					QCDCallbacks.Service( opSetAudioInfo, &ai, sizeof(AudioInfo), 0);
					aiLast = ai;
				}

				//
				// send to output
				//
				if ( !QCDCallbacks.toPlayer.OutputWrite( &wd))
					Sleep(10); // to avoid racing if output closes
			} else {
				// handle decoder error?
				Sleep(10);
			}
		}

		//
		// Decoding Complete
		//
		if ( bDecodeDone) {
			// check for seek after drain return since draincancel could have been called
			if ( QCDCallbacks.toPlayer.OutputDrain(0) && !g_bSeekNeeded) {
				QCDCallbacks.toPlayer.OutputStop(STOPFLAG_PLAYDONE);
				QCDCallbacks.toPlayer.PlayDone( (LPCSTR)(LPCWSTR)threadData->mediaReader.GetName(), 0);
			}

			if ( g_bSeekNeeded)
				bDecodeDone = FALSE;
			else
				break;
		}

		// catch pause
		while ( g_bIsPaused && !g_bDecoderThreadKill)
			Sleep(50);
	}

	// close up
	if ( pQDecoder) delete pQDecoder;

	if ( threadData) {
		delete threadData;
		*ppthreadData = NULL;
	}

	if ( pReaderStatus) delete pReaderStatus;

	_endthreadex(0);
	return 0;
}

