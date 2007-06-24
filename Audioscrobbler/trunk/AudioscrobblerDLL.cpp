//-----------------------------------------------------------------------------
//
// File:	AudioscrobblerDLL.cpp
//
// Authors:	Toke Noer (toke@noer.it)
//	Copyright (C) 2007
//
//-----------------------------------------------------------------------------
// TODO
//   Handle stream titles - InfoChanged
//   Proxy connection
//-----------------------------------------------------------------------------

#include "Precompiled.h"

#define PLUGIN_NAME L"Audioscrobbler v0.1.4"

// Project includes
//#include "ThreadTools.h"
#include "PlayerControl.h"
//#include "About.h"
#include "Config.h"
#include "AudioscrobblerDLL.h"


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
HINSTANCE       g_hInstance   = 0;
HWND            g_hwndPlayer  = NULL;
WNDPROC         g_QMPProc     = 0;

QCDModInitGen2  QMPCallbacks;
_Settings       Settings;
CLog*           log;

HANDLE            g_hASThread            = NULL;
HANDLE            g_hASThreadEndedEvent  = NULL;
ULONG             g_nASThreadId          = 0;
CRITICAL_SECTION  g_csAIPending;
BOOL              g_bIsClosing           = FALSE;

CAudioInfo* g_pAIPending = NULL;

//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		// Don't use below when we use CreateThread. See doc for CreateThread, last remark
		// DisableThreadLibraryCalls(hInst);
		g_hInstance = hInst;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------

PLUGIN_API QCDModInitGen2* GENERAL2_DLL_ENTRY_POINT()
{
	QMPCallbacks.size    = sizeof(QCDModInitGen2);
	QMPCallbacks.version = PLUGIN_API_VERSION_UNICODE;

	QMPCallbacks.toModule.Initialize = Initialize;
	QMPCallbacks.toModule.ShutDown   = ShutDown;
	QMPCallbacks.toModule.About      = About;
	QMPCallbacks.toModule.Configure  = Configure;

	return &QMPCallbacks;
}

//-----------------------------------------------------------------------------

int Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = (char*)PLUGIN_NAME;

	// Check QMP version (b117+)
	if (QMPCallbacks.Service(opGetPlayerVersion, NULL, 1, 0) < 117) {
		MessageBox(0, L"The latest beta builds of Quintessential Media Player\nis needed to run this Audioscrobbler plug-in.\n\nAt least build 117 required!",
			L"QMP:Audioscrobbler initialization error", MB_ICONSTOP);
		return FALSE;
	}

	// Initialization
	_tzset();

	g_hwndPlayer  = (HWND)QMPCallbacks.Service(opGetParentWnd, 0, 0, 0);
	if (!g_hwndPlayer)
		return false;

	LoadSettings();

	// Create log object
	WCHAR strPath[MAX_PATH] = {0};
	QMPCallbacks.Service(opGetSettingsFolder, strPath, MAX_PATH*sizeof(WCHAR), 0);
	wcscat_s(strPath, MAX_PATH, L"\\Audioscrobbler.log");
	log = new CLog(g_hwndPlayer, Settings.logMode, strPath);

	// Setup menu item in plugins menu
	// const _TCHAR* strMenu = _T("Audioscrobbler");
	// QMPCallbacks->Service(opSetPluginMenuItem, hInstance, IDM_ABREPEAT, (LONG)strMenu);

	// Subclass the player and listen for WM_PN_?
	if ((g_QMPProc = (WNDPROC)SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)QMPSubProc)) == 0) {
		log->OutputInfo(E_FATAL, _T("Failed to subclass player!"));
		return FALSE;
	}
	
	// Start curl thread
	InitializeCriticalSection(&g_csAIPending);
	g_hASThreadEndedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_hASThread = CreateThread(NULL, 0, AS_Main, NULL, 0, &g_nASThreadId);
	if (!g_hASThread)
		return FALSE; // TODO: Cleanup
	// SetThreadName(g_nASThreadId, "QMP_Audioscrobbler");

	Sleep(0);

	// return TRUE for successful initialization
	log->OutputInfo(E_DEBUG, _T("Initialize(): return true"));
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	g_QMPProc = (WNDPROC)SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)g_QMPProc);

	// Close curl thread
	g_bIsClosing = TRUE;
	PostThreadMessage(g_nASThreadId, WM_QUIT, 0, 0);
	Sleep(0);
	if (WaitForSingleObject(g_hASThreadEndedEvent, 40000) == WAIT_TIMEOUT)
		TerminateThread(g_hASThread, -1);

	CloseHandle(g_hASThread);
	CloseHandle(g_hASThreadEndedEvent);
	DeleteCriticalSection(&g_csAIPending);
	
	SaveSettings();

	delete g_pAIPending;
	delete log;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	CreateConfigDlg(g_hInstance, g_hwndPlayer);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	// CreateAboutDlg(g_hInstance, g_hwndPlayer);
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK QMPSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{ 
	switch (msg)
	{
		case WM_PN_PLAYSTARTED :
			PlayStarted();
			break;

		case WM_PN_PLAYDONE :
			PlayDone();
			break;

		case WM_PN_PLAYSTOPPED :
			PlayStopped();
			break;

		case WM_PN_PLAYPAUSED :
			PlayPaused();
			break;
		
		case WM_PN_TRACKCHANGED :
			TrackChanged();
			break;

		case WM_PN_INFOCHANGED :
			if (lparam <= 'Z')
				break;
			InfoChanged((LPCSTR)lparam);
			break;

	} // Switch

	return CallWindowProc(g_QMPProc, hwnd, msg, wparam, lparam); 
}

//-----------------------------------------------------------------------------