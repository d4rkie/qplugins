//-----------------------------------------------------------------------------
// About
//   All contact with the player and updating of the pending track here
//-----------------------------------------------------------------------------

#ifndef __PLAYERCONTROL_H_
#define __PLAYERCONTROL_H_

#include "AudioscrobblerDLL.h"
#include "Audioscrobbler.h"


//-----------------------------------------------------------------------------

// This handles next/prev track, since we receive no notification of those
bool g_bTrackIsDone = TRUE;

//-----------------------------------------------------------------------------

void PlayStarted();
void PlayStopped();
void PlayDone();

CAudioInfo* GetAudioInfo(LPCWSTR strFile);
void GetAudioInfo(LPCWSTR strFile, CAudioInfo* ai);

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

	// Only support for audio files and audio streams
	long nMediaType = QMPCallbacks.Service(opGetMediaType, NULL, -1, 0);
	if (nMediaType == DIGITAL_AUDIOFILE_MEDIA)
		g_pAIPending = GetAudioInfo(strFileUCS2);
	/*else if (nMediaType == DIGITAL_AUDIOSTREAM_MEDIA)
	{
		CAudioInfo ai;
		GetAudioInfo(&ai);
		g_pAIPending = new CAudioInfo(ai);

		// Shoutcast streams has station set in album field, so parse title
		std::string strTmp;
		strTmp.assign(ai.GetTitle());
		int nPos = strTmp.find("/", 0);
		if (nPos > 0)
			g_pAIPending->SetTitle(strTmp.substr(0, nPos).c_str());

	}*/

	if (g_pAIPending)
	{
		if (!*(g_pAIPending->GetArtist()) || !*(g_pAIPending->GetTitle()) )
		{
			delete g_pAIPending;
			g_pAIPending = NULL;
		}
		else // Send information
			PostThreadMessage(g_nASThreadId, AS_MSG_PLAY_STARTED, 0, 0);
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
void TrackChanged()
{
	log->OutputInfo(E_DEBUG, _T("TrackChanged"));

	PostThreadMessage(g_nASThreadId, AS_MSG_TRACK_CHANGED, 0, 0);
}

//-----------------------------------------------------------------------------

__inline
void InfoChanged(LPCSTR szMedianame)
{
	// if (IsStream() && wcscmp(szMedianame, g_szPlayingfile) == 0)
	//		PlayDone();
	//    PlayStarted();
	// log->OutputInfoW(E_DEBUG, L"InfoChanged: %s", (LPWSTR)szMedianame);
}

//-----------------------------------------------------------------------------

CAudioInfo* GetAudioInfo(LPCWSTR strFile)
{
	CAudioInfo* ai = new CAudioInfo();
	GetAudioInfo(strFile, ai);
	return ai;
}

void GetAudioInfo(LPCWSTR strFile, CAudioInfo* ai)
{
	const static int BUF_SIZE = 1024;
	WCHAR strUCS2[BUF_SIZE] = {0};
	long nDataSize = 0;

	long nIndex = QMPCallbacks.Service(opGetCurrentIndex, NULL, 0, 0);

	ai->SetTrackLength(QMPCallbacks.Service(opGetTrackLength, NULL, nIndex, 0));
	ai->SetStartTime();
	ai->SetRating("");

	//***********************************************
	// Get Artist, Title, Album, Tracknumber
	//***********************************************
	// Use MediaInfo interface if possible
	IQCDMediaInfo* info = (IQCDMediaInfo*)QMPCallbacks.Service(opGetIQCDMediaInfo, (void*)strFile, 0, 0);
	if (info)
	{
		if (!info->LoadFullData())
			goto cleanup;

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
		else
			ai->SetTrackNumber(0);


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

		cleanup:
		info->Release();
	}
	else
	// Use non interface version
	{
		
		QMPCallbacks.Service(opGetArtistName, strUCS2, BUF_SIZE, nIndex);
		ai->SetArtist(strUCS2);
	
		QMPCallbacks.Service(opGetTrackName, strUCS2, BUF_SIZE, nIndex);
		ai->SetTitle(strUCS2);

		QMPCallbacks.Service(opGetAlbumName, strUCS2, BUF_SIZE, nIndex);
		ai->SetAlbum(strUCS2);

		// We don't get tracknumber, since opGetTrackNum returns 1 on unknown		
		ai->SetTrackNumber(0);
	}


	//***********************************************
	// Get MusicBrainzID
	//***********************************************
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
}

//-----------------------------------------------------------------------------

#endif // __PLAYERCONTROL_H_