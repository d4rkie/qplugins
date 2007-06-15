//-----------------------------------------------------------------------------
// About
//   All contact with the player and adding to the AudioInfo queue, wil be in
//   this file and run on the main thread. Putting it in worker thread won't work!
//-----------------------------------------------------------------------------

#ifndef __PLAYERCONTROL_H_
#define __PLAYERCONTROL_H_

#include "AudioscrobblerDLL.h"
#include "Audioscrobbler.h"
#include "Log.h"


void PlayStarted();
void PlayStopped();
void PlayDone();

CAudioInfo* GetAudioInfo();
void GetAudioInfo(CAudioInfo* ai);

//-----------------------------------------------------------------------------

__inline
void PlayStarted()
{
	WCHAR strFileUCS2[MAX_PATH] = {0};
	QMPCallbacks.Service(opGetPlaylistFile, strFileUCS2, MAX_PATH, -1);
	log->OutputInfo(E_DEBUG, _T("PlayStarted : %s"), strFileUCS2);

	// Check if song has been added to queue, delete otherwise
	EnterCriticalSection(&g_csAIQueue);
	if (g_pAIPending)	{
		delete g_pAIPending;
		g_pAIPending = NULL;
	}

	g_pAIPending = GetAudioInfo();
	LeaveCriticalSection(&g_csAIQueue);

	// Send information
	PostThreadMessage(g_nASThreadId, AS_MSG_PLAY_STARTED, 0, 0);
}

//-----------------------------------------------------------------------------

__inline
void PlayStopped()
{
	log->OutputInfo(E_DEBUG, _T("PlayStopped"));

	PostThreadMessage(g_nASThreadId, AS_MSG_PLAY_STOPPED, 0, 0);
}

//-----------------------------------------------------------------------------

__inline
void PlayDone()
{
	log->OutputInfo(E_DEBUG, _T("PlayDone"));

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

CAudioInfo* GetAudioInfo()
{
	CAudioInfo* ai = new CAudioInfo();
	GetAudioInfo(ai);
	return ai;
}

void GetAudioInfo(CAudioInfo* ai)
{
	const static int BUF_SIZE = 1024;
	WCHAR strUCS2[BUF_SIZE] = {0};
	char strUTF8[BUF_SIZE] = {0};

	long nIndex = QMPCallbacks.Service(opGetCurrentIndex, NULL, 0, 0);

	ai->SetStartTime();

	QMPCallbacks.Service(opGetArtistName, strUCS2, BUF_SIZE, nIndex);
	QMPCallbacks.Service(opUCS2toUTF8, strUCS2, (long)strUTF8, BUF_SIZE);
	ai->SetArtist(strUTF8);
	
	QMPCallbacks.Service(opGetTrackName, strUCS2, BUF_SIZE, nIndex);
	QMPCallbacks.Service(opUCS2toUTF8, strUCS2, (long)strUTF8, BUF_SIZE);
	ai->SetTitle(strUTF8);

	ai->SetTrackLength(QMPCallbacks.Service(opGetTrackLength, NULL, nIndex, 0));

	QMPCallbacks.Service(opGetAlbumName, strUCS2, BUF_SIZE, nIndex);
	QMPCallbacks.Service(opUCS2toUTF8, strUCS2, (long)strUTF8, BUF_SIZE);
	ai->SetAlbum(strUTF8);

	// opGetTrackNum returns 1 even on unknown
	ai->SetTrackNumber(QMPCallbacks.Service(opGetTrackNum, NULL, nIndex, 0));

	ai->SetMusicBrainTrackId("");
	ai->SetRating("");
}

//-----------------------------------------------------------------------------

/*void CALLBACK cbSubmitSong(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
	if (g_nSubmitTimer) {
		KillTimer(NULL, g_nSubmitTimer);
		g_nSubmitTimer = 0;
	}

	if (!g_pAIPending)
		return;

	CAudioInfo* ai = g_pAIPending;
	g_pAIPending = NULL;

	// TODO: Check if song progress is where it should be, else it has been paused and we need to set a new timeout

	g_AIQueue.push_back(ai);
	log->OutputInfoA(E_DEBUG, "Song added to queue: %s/%s", ai->GetArtist(), ai->GetTitle());
}*/

#endif // __PLAYERCONTROL_H_