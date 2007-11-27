#ifndef AUDIOSCROBBLER_H_
#define AUDIOSCROBBLER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//-----------------------------------------------------------------------------
enum AS_APM_COMMANDS {
	AS_MSG_PLAY_STARTED = WM_APP+10,
	AS_MSG_PLAY_STOPPED,
	AS_MSG_PLAY_DONE,
	AS_MSG_PLAY_PAUSED,
	AS_MSG_TRACK_CHANGED,
	AS_MSG_SETTINGS_CHANGED,
	AS_MSG_OFFLINE_MODE
};

extern BOOL g_bOfflineMode;

DWORD WINAPI AS_Main(LPVOID lpParameter);

//-----------------------------------------------------------------------------


#endif // AUDIOSCROBBLER_H_