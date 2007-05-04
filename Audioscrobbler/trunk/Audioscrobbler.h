#ifndef AudioscrobblerDLL_H
#define AudioscrobblerDLL_H

#include <QCDCtrlMsgs.h>
#include <QCDModDefs.h>
#include <QCDModGeneral2.h>

//-----------------------------------------------------------------------------

struct _Settings
{
	BOOL bShowWindowOnStart;
};

//-----------------------------------------------------------------------------

extern HINSTANCE      hInstance;
extern HWND           hwndPlayer;
extern QCDModInitGen2 QMPCallbacks;
extern _Settings      Settings;

//-----------------------------------------------------------------------------
// Calls from the Player
int  Initialize(QCDModInfo *modInfo, int flags);
void Configure(int flags);
void About(int flags);
void ShutDown(int flags);

// Callbacks
LRESULT CALLBACK QMPSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif // AudioscrobblerDLL_H