//-----------------------------------------------------------------------------
//
// File:	QMPGeneralDLL.cpp
//
// SDK Authors:	Written by Paul Quinn and Richard Carlson.
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
//
// TODO:
//    Error handling on file errors
//    Stream information
//-----------------------------------------------------------------------------

#define PLUGIN_NAME L"QMP Export v0.2"

#include <time.h>

#include "PlayerSubclass.h"
#include "QMPGeneralDLL.h"
#include "ExportFunctions.h"
#include "Config.h"

#define IDM_HEAD         0x11
#define IDM_EXPORT_XML   0x12
#define IDM_EXPORT_HTML  0x13
#define IDM_CONFIGURE    0x14


HINSTANCE                 g_hInstance   = NULL;
HWND                      g_hwndPlayer  = NULL;

QCDModInitGen2            QMPCallbacks;
_Settings                 Settings;

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
	QMPCallbacks.size				= sizeof(QCDModInitGen2);
	QMPCallbacks.version			= PLUGIN_API_VERSION_UNICODE;

	QMPCallbacks.toModule.Initialize = Initialize;
	QMPCallbacks.toModule.ShutDown   = ShutDown;
	QMPCallbacks.toModule.About      = About;
	QMPCallbacks.toModule.Configure  = Configure;

	return &QMPCallbacks;
}

//-----------------------------------------------------------------------------

int Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = reinterpret_cast<char*>(PLUGIN_NAME);

	//
	// TODO: all your plugin initialization here
	//
	_tzset();
	
	g_hwndPlayer = (HWND)QMPCallbacks.Service(opGetParentWnd, 0, 0, 0);

	if (!(CPlayerSubclass::GetInstance()->SubclassPlayer(g_hwndPlayer))) {
		MessageBox(g_hwndPlayer, _T("Failed to subclass player!"), _T("Fatal init error!"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	QMPCallbacks.Service(opSetPluginMenuItem,  g_hInstance, IDM_HEAD, (LONG)L"Export");
	QMPCallbacks.Service(opSetPluginMenuState, g_hInstance, IDM_HEAD, MF_POPUP);

	QMPCallbacks.Service(opSetPluginMenuItem,  g_hInstance, IDM_EXPORT_XML,  (LONG)_T("Export XML"));
	QMPCallbacks.Service(opSetPluginMenuItem,  g_hInstance, IDM_EXPORT_HTML, (LONG)_T("Export HTML"));

	QMPCallbacks.Service(opSetPluginMenuItem,  g_hInstance, IDM_EXPORT_HTML, MF_SEPARATOR);
	QMPCallbacks.Service(opSetPluginMenuItem,  g_hInstance, IDM_CONFIGURE,  (LONG)_T("Configuration"));

	LoadSettings();

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	CPlayerSubclass::Destroy();
	QMPCallbacks.Service(opSetPluginMenuItem, g_hInstance, IDM_HEAD, 0);
	SaveSettings();
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	switch (flags)
	{
		//case IDM_HEAD :
		//	break;
		case IDM_EXPORT_XML :
			ExportXml();
			break;
		case IDM_EXPORT_HTML :
			ExportHtml();
			break;

		default :
			CreateConfigDlg(g_hInstance, g_hwndPlayer);
			break;
	}
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	MessageBox(g_hwndPlayer, _T("Plug-in by:\nToke Noer\ntoke@noer.it"), _T("About"), MB_OK | MB_ICONINFORMATION);
}

//-----------------------------------------------------------------------------