//-----------------------------------------------------------------------------
//
// File:	AudioscrobblerDLL.cpp
//
// Authors:	Toke Noer (toke@noer.it)
//	Copyright (C) 2007
//
//-----------------------------------------------------------------------------
// Changelog 0.1.7:
//   * Fix: CD's wasn't scrobbled
//   * Fix: Crash if username had special characters
//   * Fix: Trying to send too often when connection is bad
//   * Fix: Offline mode didn't work, if the player had handshaken already.
//   * Updated CURL Lib (memory leak fix)
//
// Known bugs:
//   Don't send info when encoding
//
// TODO / Wishlist
//   Custom send time. (Send at 50%->100% slider)
//   Handle stream titles - InfoChanged
//   Proxy connection
//   Win9x support
//-----------------------------------------------------------------------------

#include "Precompiled.h"

#define PLUGIN_NAME L"Audioscrobbler v0.1.7"

// Project includes
//#include "ThreadTools.h"
#include "PlayerControl.h"
#include "About.h"
#include "Config.h"
#include "AudioscrobblerDLL.h"


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define IDM_CONFIG        1
#define IDM_HEAD          2
#define IDM_OFFLINE_MODE  3

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
QCDModInitGen2    QMPCallbacks;
_Settings         Settings;
CLog*             log                    = NULL;

HINSTANCE         g_hInstance            = 0;
HWND              g_hwndPlayer           = NULL;
WNDPROC           g_QMPProc              = 0;

HANDLE            g_hASThread            = NULL;
HANDLE            g_hASThreadEndedEvent  = NULL;
ULONG             g_nASThreadId          = 0;
CRITICAL_SECTION  g_csAIPending;
BOOL              g_bIsClosing           = FALSE;

CAudioInfo*       g_pAIPending           = NULL;

//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		#ifdef _DEBUG
			_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
			//_CrtSetBreakAlloc(398);
		#endif

		// Don't use DisableThreadLibraryCalls when we use CreateThread. See doc for CreateThread, last remark
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

	// Check QMP version (b118+)
	if (QMPCallbacks.Service(opGetPlayerVersion, NULL, 1, 0) < 118) {
		MessageBox(0, L"The latest beta builds of Quintessential Media Player\nis needed to run this Audioscrobbler plug-in.\n\nAt least build 118 required!",
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
	QMPCallbacks.Service(opSetPluginMenuItem,  g_hInstance, IDM_HEAD,         (LONG)_T("Audioscrobbler"));
	QMPCallbacks.Service(opSetPluginMenuState, g_hInstance, IDM_HEAD,         MF_POPUP);
	QMPCallbacks.Service(opSetPluginMenuItem,  g_hInstance, IDM_CONFIG,       (LONG)_T("Configuration"));
	QMPCallbacks.Service(opSetPluginMenuItem,  g_hInstance, IDM_OFFLINE_MODE, (LONG)_T("Work Offline"));

	// Start curl thread
	InitializeCriticalSection(&g_csAIPending);
	g_hASThreadEndedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_hASThread = CreateThread(NULL, 0, AS_Main, NULL, 0, &g_nASThreadId);
	if (!g_hASThread)
		return FALSE; // TODO: Cleanup
	// SetThreadName(g_nASThreadId, "QMPAudioscrobbler");

	Sleep(0);

	// Subclass the player and listen for WM_PN_?
	if ((g_QMPProc = (WNDPROC)SetWindowLong(g_hwndPlayer, GWL_WNDPROC, (LONG)QMPSubProc)) == 0) {
		log->OutputInfo(E_FATAL, _T("Failed to subclass player!\nError code: %u"), GetLastError());
		return FALSE;
	}

	if (Settings.bFirstRun) {
		MessageBox(g_hwndPlayer, _T("This is the first time you run the Audioscrobbler plug-in.\n\nPlease provide your Audioscrobbler username and password in the following configuration screen."),
			_T("Audioscrobbler"), MB_ICONINFORMATION);
		Configure(0);
		Settings.bFirstRun = FALSE;
	}

	// return TRUE for successful initialization
	log->OutputInfo(E_DEBUG, _T("Initialize() : return true"));
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
	
	// QCDCallbacks.Service(opSafeWait, hMutex, 0, 0);	
	if (WaitForSingleObject(g_hASThreadEndedEvent, 35000) == WAIT_TIMEOUT) {
		TerminateThread(g_hASThread, -1);
		log->OutputInfo(E_DEBUG, _T("ShutDown : AS thread terminated!"));
	}
	else
		log->OutputInfo(E_DEBUG, _T("ShutDown : AS thread ended successfully"));

	CloseHandle(g_hASThread);
	CloseHandle(g_hASThreadEndedEvent);
	DeleteCriticalSection(&g_csAIPending);

	log->OutputInfo(E_DEBUG, _T("ShutDown : Saving settings"));
	SaveSettings();

	log->OutputInfo(E_DEBUG, _T("ShutDown : Deleting remaining objects"));
	delete g_pAIPending;
	delete log;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	switch (flags)
	{
		case IDM_HEAD :
			break;
		case IDM_OFFLINE_MODE :
			if (g_bOfflineMode) {
				log->OutputInfo(E_DEBUG, _T("Swiching to online mode"));
				QMPCallbacks.Service(opSetPluginMenuState,  g_hInstance, IDM_OFFLINE_MODE, MF_UNCHECKED);
			}
			else {
				log->OutputInfo(E_DEBUG, _T("Swiching to offline mode"));
				MessageBox(g_hwndPlayer, _T("Audioscrobbler is now running in offline mode\nand it will not send song information."), 
					_T("Audioscrobbler"), MB_ICONEXCLAMATION);
				QMPCallbacks.Service(opSetPluginMenuState,  g_hInstance, IDM_OFFLINE_MODE, MF_CHECKED);
			}
			// Since we can "control" this message, use it for debugging
			if (!PostThreadMessage(g_nASThreadId, AS_MSG_OFFLINE_MODE, 0, 0)) {
				log->OutputInfo(E_FATAL, _T("Failed to post thread message!\nError code: %u"), GetLastError());
			}
			else
				log->OutputInfo(E_DEBUG, _T("PostThreadMessage successfull"));


			break;

		case IDM_CONFIG :
		default :
			CreateConfigDlg(g_hInstance, g_hwndPlayer);
			break;
	}
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	CreateAboutDlg(g_hInstance, g_hwndPlayer);
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

		/*case WM_PN_INFOCHANGED :
			if (lparam <= 'Z')
				break;
			InfoChanged((LPCSTR)lparam);
			break;*/

	} // Switch

	return CallWindowProc(g_QMPProc, hwnd, msg, wparam, lparam); 
}

//-----------------------------------------------------------------------------

