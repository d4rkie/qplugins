//-----------------------------------------------------------------------------
// About
//   All contact with the player and updating of the pending track here
//-----------------------------------------------------------------------------

#ifndef __PLAYERCONTROL_H_
#define __PLAYERCONTROL_H_

#include "AudioscrobblerDLL.h"
#include "Audioscrobbler.h"


//-----------------------------------------------------------------------------
static const int CD_WAIT_TIME = 1000;

bool      g_bTrackIsDone       = TRUE; // This handles next/prev track, since we receive no notification of those
UINT_PTR  g_TrackChangedTimer  = 0;

//-----------------------------------------------------------------------------

void PlayStarted();
void PlayStopped();
void PlayDone();
void PlayPaused();
void TrackChanged();
void InfoChanged(LPCSTR szMedianame);

void CALLBACK cbTrackChanged(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime);

CAudioInfo* GetAudioInfo(LPCWSTR strFile);
BOOL GetAudioInfo(LPCWSTR strFile, CAudioInfo* ai);

//-----------------------------------------------------------------------------

__inline
void PlayStarted()
{
	WCHAR strFileUCS2[MAX_PATH] = {0};
	QMPCallbacks.Service(opGetPlaylistFile, strFileUCS2, MAX_PATH*sizeof(WCHAR), -1);
	log->OutputInfo(E_DEBUG, _T("PlayStarted : %s"), strFileUCS2);

	if (!g_bTrackIsDone)
		PlayDone();
	Sleep(0);
	g_bTrackIsDone = FALSE;

	// Check if song has been added to queue, delete otherwise
	EnterCriticalSection(&g_csAIPending);

	if (g_pAIPending)	{
		delete g_pAIPending;
		g_pAIPending = NULL;
	}

	g_pAIPending = GetAudioInfo(strFileUCS2);

	// Validate song info and send message to AS thread if valid
	if (g_pAIPending)
	{
		// Don't submit songs shorter than 30 sec (exept if they have a MBID)
		int nTrackLen = g_pAIPending->GetTrackLength();
		BOOL bArtistIsEmpty = g_pAIPending->IsEmpty(g_pAIPending->GetArtist());
		BOOL bTitleIsEmpty  = g_pAIPending->IsEmpty(g_pAIPending->GetTitle());
		BOOL bMBIdIsEmpty   = g_pAIPending->IsEmpty(g_pAIPending->GetMusicBrainTrackId());

		if ( (nTrackLen > 30 || (nTrackLen > 0 && !bMBIdIsEmpty))
				&& !bArtistIsEmpty && !bTitleIsEmpty)
		{
			// Send information
			PostThreadMessage(g_nASThreadId, AS_MSG_PLAY_STARTED, 0, 0);
		}
		else {
			log->OutputInfo(E_DEBUG, _T("PlayStarted : Required song info not set - File is not scrobbled"));
			delete g_pAIPending;
			g_pAIPending = NULL;
		}			
	}

	LeaveCriticalSection(&g_csAIPending);
}

//-----------------------------------------------------------------------------

__inline
void PlayStopped()
{
	log->OutputInfo(E_DEBUG, _T("PlayStopped"));

	g_bTrackIsDone = TRUE;
	PostThreadMessage(g_nASThreadId, AS_MSG_PLAY_STOPPED, 0, 0);
}

//-----------------------------------------------------------------------------

__inline
void PlayDone()
{
	log->OutputInfo(E_DEBUG, _T("PlayDone"));

	g_bTrackIsDone = TRUE;
	PostThreadMessage(g_nASThreadId, AS_MSG_PLAY_DONE, 0, 0);
}

//-----------------------------------------------------------------------------

__inline
void PlayPaused()
{
	log->OutputInfo(E_DEBUG, _T("PlayPaused"));
	
	PostThreadMessage(g_nASThreadId, AS_MSG_PLAY_PAUSED, 0, 0);
}

//-----------------------------------------------------------------------------

__inline
void InfoChanged(LPCSTR szMedianame)
{
	// Is commented out in the subclassing proc
	// if (IsStream() && wcscmp(szMedianame, g_szPlayingfile) == 0)
	//		PlayDone();
	//    PlayStarted();
	// log->OutputInfoW(E_DEBUG, L"InfoChanged: %s", (LPWSTR)szMedianame);
}

//-----------------------------------------------------------------------------

__inline
void TrackChanged()
{
	log->OutputInfo(E_DEBUG, _T("TrackChanged"));

	if (g_TrackChangedTimer) {
		KillTimer(NULL, g_TrackChangedTimer);
		g_TrackChangedTimer = 0;
	}
	g_TrackChangedTimer = SetTimer(NULL, NULL, CD_WAIT_TIME, cbTrackChanged);
}

//-----------------------------------------------------------------------------

void CALLBACK cbTrackChanged(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
	if (g_TrackChangedTimer) {
		KillTimer(NULL, g_TrackChangedTimer);
		g_TrackChangedTimer = 0;
	}

	PlayStarted();
}

//-----------------------------------------------------------------------------

CAudioInfo* GetAudioInfo(LPCWSTR strFile)
{
	CAudioInfo* ai = new CAudioInfo();
	if (GetAudioInfo(strFile, ai))
		return ai;
	else
		delete ai;

	return NULL;
}

BOOL GetAudioInfo(LPCWSTR strFile, CAudioInfo* ai)
{
	const static int BUF_SIZE = 1024;
	WCHAR strUCS2[BUF_SIZE] = {0};
	long nDataSize = 0;
	IQCDMediaInfo* info = NULL;

	
	// Only support for audio files and CD atm.
	long nMediaType = QMPCallbacks.Service(opGetMediaType, NULL, -1, 0);
	if (nMediaType != DIGITAL_AUDIOFILE_MEDIA && nMediaType != CD_AUDIO_MEDIA) {
		log->OutputInfo(E_DEBUG, _T("PlayStarted : File not a CD or digital audio file - File is not scrobbled"));
		return FALSE;
	}
	/*if (nMediaType == DIGITAL_AUDIOSTREAM_MEDIA)
	{
		// Shoutcast streams has station set in album field, so parse title
		std::string strTmp;
		strTmp.assign(ai.GetTitle());
		int nPos = strTmp.find("/", 0);
		if (nPos > 0)
			g_pAIPending->SetTitle(strTmp.substr(0, nPos).c_str());

	}*/


	long nIndex = QMPCallbacks.Service(opGetCurrentIndex, NULL, 0, 0);

	long nTrackLen = QMPCallbacks.Service(opGetTrackLength, NULL, nIndex, 0); // Get seconds
	if (nTrackLen <= 0) {
		log->OutputInfo(E_DEBUG, _T("GetAudioInfo : Tracklength=%u"), nTrackLen);
		return FALSE;
	}

	ai->SetTrackNumber(0);
	ai->SetTrackLength(nTrackLen);	
	ai->SetStartTime();
	ai->SetRating("");

	//*************************************************************************
	// Get Artist, Title, Album, Tracknumber
	//*************************************************************************
	// Use MediaInfo interface if possible

	if (nMediaType != CD_AUDIO_MEDIA && 
		(info = (IQCDMediaInfo*)QMPCallbacks.Service(opGetIQCDMediaInfo, (void*)strFile, 0, 0)) )
	{
		if (!info->LoadFullData())
			goto cleanup;

		log->OutputInfo(E_DEBUG, _T("GetAudioInfo : Retrieving info through interface"));

		// Artist
		nDataSize = BUF_SIZE;
		info->GetInfoByName(QCDInfo_ArtistTrack, strUCS2, &nDataSize);
		ai->SetArtist(strUCS2);

		// Title
		nDataSize = BUF_SIZE;
		info->GetInfoByName(QCDInfo_TitleTrack, strUCS2, &nDataSize);
		ai->SetTitle(strUCS2);

		// Album
		nDataSize = BUF_SIZE;
		info->GetInfoByName(QCDInfo_TitleAlbum, strUCS2, &nDataSize);
		ai->SetAlbum(strUCS2);
	
		// Tracknumber
		nDataSize = BUF_SIZE;
		if ( info->GetInfoByName(QCDInfo_TrackNumber, strUCS2, &nDataSize) ) {
			WCHAR* pData = strUCS2;
			int i = 0;
			for ( ; *pData && *pData != '/'; i++)
				strUCS2[i] = *pData++;
			strUCS2[i] = NULL;

			ai->SetTrackNumber(strUCS2);
		}


		cleanup:
			info->Release();
	}
	else
	{  // Use non interface version
		log->OutputInfo(E_DEBUG, _T("GetAudioInfo : Retrieving info with old method"));
		
		QMPCallbacks.Service(opGetArtistName, strUCS2, BUF_SIZE, nIndex);
		ai->SetArtist(strUCS2);
	
		QMPCallbacks.Service(opGetTrackName, strUCS2, BUF_SIZE, nIndex);
		ai->SetTitle(strUCS2);

		QMPCallbacks.Service(opGetAlbumName, strUCS2, BUF_SIZE, nIndex);
		ai->SetAlbum(strUCS2);

		// We don't get tracknumber for mp3's etc, since opGetTrackNum returns 1 on unknown
		if (nMediaType == CD_AUDIO_MEDIA) {
			long nTrackNr = QMPCallbacks.Service(opGetTrackNum, NULL, nIndex, 0);
			ai->SetTrackNumber(nTrackNr);
		}
	}


	//*************************************************************************
	// Get MusicBrainzID
	//*************************************************************************
	IQCDTagInfo* tag = (IQCDTagInfo*)QMPCallbacks.Service(opGetIQCDTagInfo, (void*)strFile, 0, 0);
	if ( tag )
	{
		QTAGDATA_TYPE tagType;
		BYTE tagData[128] = {0};
		DWORD tagLength = 128;

		tag->ReadFromFile(TAG_DEFAULT);

		if (tag->GetTagDataByName(L"MUSICBRAINZ_TRACKID", &tagType, tagData, &tagLength, 0))
			ai->SetMusicBrainTrackId((LPCWSTR)tagData);
		
		tag->Release();
	}
	
	return TRUE;
}

//-----------------------------------------------------------------------------

#endif // __PLAYERCONTROL_H_



		/*int i = 0;
		long nNameLen = 1024;
		long nValueLen = 1024;
		WCHAR szName[1024];
		WCHAR szValue[1024];
		while ( info->GetInfoByIndex(i++, szName, &nNameLen, szValue, &nValueLen) )
		{
			nNameLen = 1024;
			nValueLen = 1024;
		}*/