#ifndef __AudioscrobblerDLL_H
#define __AudioscrobblerDLL_H

#include "curl\include\curl\curl.h"
#include <QString.h>

#include <QCDCtrlMsgs.h>
#include <QCDModDefs.h>
#include <QCDModGeneral2.h>

#include "Log.h"
#include "AudioInfo.h"

//-----------------------------------------------------------------------------

struct _Settings
{
	LogMode logMode;
	QString strUsername;
	char strPassword[33];
};

//-----------------------------------------------------------------------------

extern HINSTANCE      g_hInstance;
extern HWND           g_hwndPlayer;
extern QCDModInitGen2 QMPCallbacks;
extern _Settings      Settings;
extern CLog*          log;

extern HANDLE            g_hASThreadEndedEvent;
extern ULONG             g_nASThreadId;
extern CRITICAL_SECTION  g_csAIQueue;
extern BOOL              g_bIsClosing;

extern CAudioInfo* g_pAIPending;
extern std::deque<CAudioInfo*> g_AIQueue;


//-----------------------------------------------------------------------------
// Calls from the Player
int  Initialize(QCDModInfo *modInfo, int flags);
void Configure(int flags);
void About(int flags);
void ShutDown(int flags);

// Callbacks
LRESULT CALLBACK QMPSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif // __AudioscrobblerDLL_H