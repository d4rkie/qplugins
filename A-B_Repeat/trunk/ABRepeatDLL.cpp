//-----------------------------------------------------------------------------
//
// File:	QCDGeneralDLL.cpp
//
// About:	See QCDGeneralDLL.h
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
// Bugs: 


#define PLUGIN_NAME "A-B Repeat v0.4"


#include <TCHAR.h>


#include "resource.h"
#include "MainDialog.h"
#include "Config.h"
#include "ABRepeatDLL.h"

//-----------------------------------------------------------------------------

HINSTANCE		hInstance        = 0;
HWND			hwndPlayer       = NULL;

QCDService*     QCDCallbacks     = NULL;
QCDModInitGen*  ModInitGen1      = NULL;
QCDModInitGen2* ModInitGen2      = NULL;
BOOL            g_IsNewApi       = FALSE;
_Settings       Settings;

WNDPROC			QCDProc          = 0;
HWND			hwndMainDlg      = NULL;
HWND			hwndQCDWA        = NULL;

BOOL m_bRunning  = FALSE;
LONG m_iStartPos = 0;
LONG m_iEndPos   = 0;
BYTE m_Status    = STATUS_NOMARK;


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

PLUGIN_API BOOL GENERALDLL_ENTRY_POINT(QCDModInitGen *ModInit, QCDModInfo *ModInfo)
{
	ModInit->size               = sizeof(QCDModInitGen); // Old API sizeof
	ModInit->version            = PLUGIN_API_VERSION;

	ModInit->toModule.ShutDown  = ShutDown;
	ModInit->toModule.About     = About;
	ModInit->toModule.Configure = Configure;

	ModInitGen1 = ModInit;
	g_IsNewApi = false;
	Initialize(ModInfo, 0);
	return TRUE; // return TRUE for successful initialization
}

PLUGIN_API QCDModInitGen2* GENERAL2_DLL_ENTRY_POINT()
{
	ModInitGen2 = new QCDModInitGen2;

	ModInitGen2->size				= sizeof(QCDModInitGen2);
	ModInitGen2->version			= PLUGIN_API_VERSION;

	ModInitGen2->toModule.Initialize= Initialize;
	ModInitGen2->toModule.ShutDown	= ShutDown;
	ModInitGen2->toModule.About		= About;
	ModInitGen2->toModule.Configure	= Configure;

	g_IsNewApi = true;
	return ModInitGen2;
}

//-----------------------------------------------------------------------------

int Initialize(QCDModInfo *modInfo, int flags)
{
	// Setup our QCD/QMP callback
	QCDCallbacks = new QCDService;
	if (g_IsNewApi)
		QCDCallbacks->Service = ModInitGen2->Service;
	else
		QCDCallbacks->Service = ModInitGen1->Service;
	modInfo->moduleString = PLUGIN_NAME;

	// Initialization
	hwndPlayer  = (HWND)QCDCallbacks->Service(opGetParentWnd, 0, 0, 0);
	if (!hwndPlayer)
		return false;

	// Setup menu item in plugins menu
	const _TCHAR* strMenu = _T("A-B Repeat - Show window");
	QCDCallbacks->Service(opSetPluginMenuItem, hInstance, IDM_ABREPEAT, (LONG)strMenu);

	// Subclass the player and listen for WM_PN_?
	if ((QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDSubProc)) == 0) {
		MessageBox(hwndPlayer, "Failed to subclass player!", "Error!", MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	LoadSettings();
	if (Settings.bShowWindowOnStart)
		CreateMainDlg();
	
	return TRUE; // return TRUE for successful initialization
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDProc);
	QCDCallbacks->Service(opSetPluginMenuItem, hInstance, IDM_ABREPEAT, 0);
	SaveSettings();
	if (hwndMainDlg != NULL)
		SendMessage(hwndMainDlg, WM_CLOSE, 0, 0);
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	if (hwndMainDlg == NULL)
		CreateMainDlg();
	else
		ShowWindow(hwndMainDlg, SW_SHOW);
	
	if (flags != IDM_ABREPEAT)
		CreateConfigDlg(hInstance, hwndPlayer);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	MessageBox(hwndPlayer, _T("A-B Repeat by:\n\nToke Noer\ntoke@noer.it"), _T("About"), MB_OK | MB_ICONINFORMATION);
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{ 
	switch (msg) {
		case WM_PN_PLAYBACKPROGRESS :
		{
			// Monitor the playback progress and rewind when we reach mark B == m_iEndPos
			if (m_Status == STATUS_ABMODE)
			{
				if (hwndQCDWA == NULL)
					hwndQCDWA = FindWindow("Winamp v1.x", NULL);
				if (hwndQCDWA) {
					long iPosition = SendMessage(hwndQCDWA, WM_USER, 0, IPC_GETOUTPUTTIME);

					if (iPosition >= m_iEndPos)
						QCDCallbacks->Service(opSetSeekPosition, NULL, m_iStartPos, 1);		// in ms
				}
			}
			break;
		}
	} // Switch

	return CallWindowProc(QCDProc, hwnd, msg, wparam, lparam); 
}

//-----------------------------------------------------------------------------