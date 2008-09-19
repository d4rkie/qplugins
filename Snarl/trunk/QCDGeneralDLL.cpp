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
// Plug-in written by Toke Noer : toke@noer.it
//-----------------------------------------------------------------------------
// http://www.k23productions.com/haiku/snarl/dev/
// http://www.quinnware.com/forum/showthread.php?t=5594
//-----------------------------------------------------------------------------
// Todo:
//  - Snarl doesn't show popups on track change on internet radio stations.
//-----------------------------------------------------------------------------

static const wchar_t PLUGIN_NAME[] = L"Snarl v2.0.0";

#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")


#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#include <TCHAR.H>
#include <QString.h>

#include <IQCDTagInfo.h>

#include "CoverArt.h"
#include "SnarlHelper.h"
#include "Settings.h"
#include "Config.h"

#include "SnarlComThread.h"

#include "QCDGeneralDLL.h"


static const wchar_t PLUGIN_NAME_FAILED[] = L"Snarl plug-in failed to load";


QCDModInitGen2    QCDCallbacks;
PluginServiceFunc Service;
HINSTANCE         hInstance       = NULL;
HWND              hwndPlayer      = NULL;
WNDPROC           QCDProc         = NULL;
SnarlInterface*   snarlInterface  = NULL;

_Settings         Settings;
QString           g_strDefaultIcon;

HANDLE            g_hThread = NULL;
DWORD             g_nThreadId = 0;

long              g_nConfigPageId = 0;
LONG32            g_nSnarlGlobalMessage = 0;
LONG32            g_nSnarlVersion = 0;


//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		#ifdef _DEBUG
			_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
			//_CrtSetBreakAlloc(398);
		#endif

		// DisableThreadLibraryCalls(hInst); // Don't disable if static link against CRT
		hInstance = hInst;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------

PLUGIN_API QCDModInitGen2* GENERAL2_DLL_ENTRY_POINT()
{
	QCDCallbacks.size				= sizeof(QCDModInitGen2);
	QCDCallbacks.version			= PLUGIN_API_VERSION_UNICODE;

	QCDCallbacks.toModule.Initialize	= Initialize;
	QCDCallbacks.toModule.ShutDown		= ShutDown;
	QCDCallbacks.toModule.About			= About;
	QCDCallbacks.toModule.Configure		= Configure;

	return &QCDCallbacks;
}

//----------------------------------------------------------------------------

int Initialize(QCDModInfo *modInfo, int flags)
{
	LPCTSTR STR_ERROR_TITLE = L"QMP:Snarl initialization error";

	modInfo->moduleString = (char*)PLUGIN_NAME;
	Service = QCDCallbacks.Service;
	hwndPlayer = (HWND)Service(opGetParentWnd, 0, 0, 0);

	// Check Windows
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (osvi.dwMajorVersion < 5) {
		modInfo->moduleString = (char*)PLUGIN_NAME_FAILED;
		MessageBox(0, L"This version of Snarl plug-in for QMP will only run on Windows 2000 or later!", STR_ERROR_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	// Check QMP version (b120+)
	if (Service(opGetPlayerVersion, NULL, 1, 0) < 120) {
		modInfo->moduleString = (char*)PLUGIN_NAME_FAILED;
		MessageBox(0, L"The latest beta builds of Quintessential Media Player\nis needed to run this Snarl plug-in.\n\nAt least build 120 required!", STR_ERROR_TITLE, MB_ICONSTOP);
		return FALSE;
	}


	LoadSettings();
	if (Settings.bStartSnarl && !snarlInterface->GetSnarlWindow()) {
		StartSnarl();
		Sleep(0);
		for (int i = 0; i < 20 && snarlInterface->GetSnarlWindow() == NULL; i++) {
			Sleep(100); // Watch sleeping for too long, we are blocking the main QMP thread !
		}
	}
	CoverArtInitialize();

	/////////////////////////////////////////////////////////////////////////////
	// Create Snarl object
	snarlInterface = new SnarlInterface();

	/////////////////////////////////////////////////////////////////////////////
	// Create helper thread
	g_hThread = CreateThread(NULL, 0, SnarlCom_Main, NULL, 0, &g_nThreadId);
	if (!g_hThread) {
		modInfo->moduleString = (char*)PLUGIN_NAME_FAILED;
		MessageBox(hwndPlayer, _T("Failed to create Snarl helper thread.\nSnarl for QMP will not continue to load!"), STR_ERROR_TITLE, 0);
		return FALSE;
	}
	Sleep(0);

	/////////////////////////////////////////////////////////////////////////////
	// Subclass the player and listen for WM_PN_? and WM_REGISTER_MSG
	if ((QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDSubProc)) == 0) {
		modInfo->moduleString = (char*)PLUGIN_NAME_FAILED;
		MessageBox(hwndPlayer, _T("Failed to subclass player!"), STR_ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	/////////////////////////////////////////////////////////////////////////////
	// Create config pages
	PluginPrefPage ppp;
	ppp.struct_size   = sizeof(PluginPrefPage);
	ppp.hModule       = hInstance;
	ppp.lpTemplate    = MAKEINTRESOURCE(IDD_CONFIG2);
	ppp.lpDialogFunc  = ConfigDlgProc;
	ppp.lpDisplayText = _T("Snarl notification");
	ppp.nCategory     = PREFPAGE_CATEGORY_GENERAL;
	ppp.hModuleParent = NULL;
	ppp.groupID       = 0;
	ppp.createParam   = 0;

	g_nConfigPageId   = Service(opSetPluginPage, (void*)&ppp, 0, 0);


	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags)
{
	QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDProc);

	/////////////////////////////////////////////////////////////////////////////
	// End helper thread
	PostThreadMessage(g_nThreadId, WM_QUIT, 0, 0);
	if (WaitForSingleObject(g_hThread, 2000) == WAIT_TIMEOUT) { // QCDCallbacks.Service(opSafeWait, g_hThread, 10000, 0)
		TerminateThread(g_hThread, -1);
		OutputDebugInfo(_T("Thread ended by TerminateThread!"));
	}
	else
		OutputDebugInfo(_T("Thread exited cleanly."));

	CloseHandle(g_hThread);
	g_hThread = NULL;
	g_nThreadId = 0;

	/////////////////////////////////////////////////////////////////////////////

	delete snarlInterface;
	snarlInterface = NULL;

	Service(opRemovePluginPage, hInstance, g_nConfigPageId, 0);

	CoverArtShutdown();
	SaveSettings();
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	Service(opShowPluginPage, hInstance, g_nConfigPageId, 0);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
#if 1
	QString strAbout = PLUGIN_NAME;
	strAbout.AppendUnicode(L"\n\nPlug-in by:\nToke Noer (toke@noer.it)");
	
	MessageBox(hwndPlayer, strAbout , _T("About"), MB_OK | MB_ICONINFORMATION);
#else
	Test();
#endif 
}

//-----------------------------------------------------------------------------


LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{
	if (msg == g_nSnarlGlobalMessage && g_nSnarlGlobalMessage > 0)
		PostThreadMessage(g_nThreadId, g_nSnarlGlobalMessage, wparam, 0);

	switch (msg)
	{
		case WM_PN_TRACKCHANGED :
			PostThreadMessage(g_nThreadId, WM_PN_TRACKCHANGED, 0, 0);
			break;

		case WM_PN_PLAYSTARTED :
			PostThreadMessage(g_nThreadId, WM_PN_PLAYSTARTED, 0, 0);
			break;

	}

	return CallWindowProc(QCDProc, hwnd, msg, wparam, lparam);
}

//-----------------------------------------------------------------------------

void OutputDebugInfo(const WCHAR* strDisplay, ...)
{
#ifndef _DEBUG
	if (!Settings.bDebug) return;
#endif

	const static size_t STR_CHARS = 512;
	WCHAR strLog[STR_CHARS] = {0};

	for (int i = 0; i < 11; i++)
		strLog[i] = L"QMP:Snarl: "[i];

	va_list args;
	va_start(args, strDisplay);
	int nCopiedChars = vswprintf_s(strLog + 11, STR_CHARS-12, strDisplay, args);
	va_end(args);

	strLog[nCopiedChars+11] = L'\n';
	strLog[nCopiedChars+12] = NULL;
	
	OutputDebugStringW(strLog);
}


//-----------------------------------------------------------------------------


void Test()
{
	/*LPCTSTR str1 = snarl->snGetAppPath();
	if (str1) {
		MessageBox(hwndPlayer, str1, _T("Test"), 0);
		delete [] str1;
	}
	str1 = snarl->snGetIconsPath();
	if (str1) {
		MessageBox(hwndPlayer, str1, _T("Test"), 0);
		delete [] str1;
	}

	snarl->snRegisterAlert("Quintessential Media Player", "Test alerts");
	LONG32 id = snarl->snShowMessage("Test", "SetTimeout=4", 4);
	if (id == 0)
		return;
	Sleep(2000);
	snarl->snUpdateMessage(id, "Test update", "SetTimeout=10");
	snarl->snSetTimeout(id, 10);*/
}