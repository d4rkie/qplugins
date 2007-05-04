//-----------------------------------------------------------------------------
//
// File:	Audioscrobbler.cpp
//
// Authors:	Toke Noer (toke@noer.it)
//	Copyright (C) 2007
//
//-----------------------------------------------------------------------------
// Version history:
//-----------------------------------------------------------------------------
// Todo:
//-----------------------------------------------------------------------------
// Bugs:
//-----------------------------------------------------------------------------

#define PLUGIN_NAME "Audioscrobbler v0.1.1"

// System includes
#include <TCHAR.h>
#include <Windows.h>

#include "curl\include\curl\curl.h"
//#include "curl\include\curl\easy.h"

// Project includes
#include "About.h"
#include "Config.h"
#include "Audioscrobbler.h"


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
HINSTANCE       hInstance     = 0;
HWND            hwndPlayer    = NULL;
WNDPROC         QMPProc       = 0;
QCDModInitGen2  QMPCallbacks;
_Settings       Settings;

//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInst);
		hInstance = hInst;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------

PLUGIN_API QCDModInitGen2* GENERAL2_DLL_ENTRY_POINT()
{
	QMPCallbacks.size    = sizeof(QCDModInitGen2);
	QMPCallbacks.version = PLUGIN_API_VERSION_NTUTF8;

	QMPCallbacks.toModule.Initialize = Initialize;
	QMPCallbacks.toModule.ShutDown   = ShutDown;
	QMPCallbacks.toModule.About      = About;
	QMPCallbacks.toModule.Configure  = Configure;

	return &QMPCallbacks;
}

//-----------------------------------------------------------------------------

int Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = PLUGIN_NAME;

	// Initialization
	hwndPlayer  = (HWND)QMPCallbacks.Service(opGetParentWnd, 0, 0, 0);
	if (!hwndPlayer)
		return false;

	// Setup menu item in plugins menu
	// const _TCHAR* strMenu = _T("Audioscrobbler");
	// QMPCallbacks->Service(opSetPluginMenuItem, hInstance, IDM_ABREPEAT, (LONG)strMenu);

	// Subclass the player and listen for WM_PN_?
	if ((QMPProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QMPSubProc)) == 0) {
		MessageBox(hwndPlayer, _T("Failed to subclass player!"), _T("Error!"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	LoadSettings();

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_WIN32);
	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_cleanup(curl);
	}
	
	curl_global_cleanup();

	
	return TRUE; // return TRUE for successful initialization
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	QMPProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QMPProc);
	// QMPCallbacks->Service(opSetPluginMenuItem, hInstance, IDM_ABREPEAT, 0);
	SaveSettings();
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	CreateConfigDlg(hInstance, hwndPlayer);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	CreateAboutDlg(hInstance, hwndPlayer);
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK QMPSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{ 
	switch (msg) {
		case WM_PN_PLAYBACKPROGRESS :
		{
			break;
		}
	} // Switch

	return CallWindowProc(QMPProc, hwnd, msg, wparam, lparam); 
}

//-----------------------------------------------------------------------------