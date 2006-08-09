//-----------------------------------------------------------------------------
//
// File:	QCDEncodeDLL.cpp
//
// About:	See QCDOutputDLL.h
//
// Authors:	Written by Paul Quinn
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

#include "stdafx.h"

#include ".\qmpclienc.h"

#include "ParentDlg.h"
#include "AboutDlg.h"

#include "QMPTagService.h"

#define PLUGIN_NAME "CLI Encoder"
#define PLUGIN_FULL_NAME "Commandline Encoder"
#define PLUGIN_VERSION "v1.5"

HINSTANCE		hInstance, hBrandInstance;
HWND			hwndParent;
QCDModInitEnc	QCDCallbacks;

CString		g_strPath;
CString		g_strParameter;
CString		g_strExtension;
QCLIEncoder	g_cliEnc;
BOOL		g_bDoTag;
BOOL		g_bNoWAVHeader;
BOOL		g_bShowConsole;
TCHAR		g_szEPFile[MAX_PATH];

int	prefPageID;
CString g_strSrc, g_strDst, g_strExt;

CParentDlg * g_pdlgParent;

INT_PTR CALLBACK PPPDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		hInstance = hInst;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------

PLUGIN_API QCDModInitEnc* ENCODEDLL_ENTRY_POINT()
{
	QCDCallbacks.version = PLUGIN_API_VERSION;

	QCDCallbacks.toModule.Initialize		= Initialize;
	QCDCallbacks.toModule.ShutDown			= ShutDown;
	QCDCallbacks.toModule.Open				= Open;
	QCDCallbacks.toModule.Write				= Write;
	QCDCallbacks.toModule.Drain				= Drain;
	QCDCallbacks.toModule.Complete			= Complete;
	QCDCallbacks.toModule.Configure			= NULL; //Configure;
	QCDCallbacks.toModule.About				= About;

	return &QCDCallbacks;
}

//-----------------------------------------------------------------------------

BOOL Initialize(QCDModInfo *modInfo, int flags)
{
	char inifile[MAX_PATH];

	modInfo->moduleString = PLUGIN_FULL_NAME " " PLUGIN_VERSION;
	modInfo->moduleExtensions = PLUGIN_NAME;

	hwndParent = (HWND)QCDCallbacks.Service( opGetParentWnd, 0, 0, 0);

	// load settings
	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	char value[MAX_PATH];
	GetPrivateProfileString( PLUGIN_NAME, _T("Path"), _T("LAME.EXE"), value, MAX_PATH, inifile);
	g_strPath = value;
	GetPrivateProfileString( PLUGIN_NAME, _T("Parameter"), _T("-V 2 --vbr-new - %d"), value, MAX_PATH, inifile);
	g_strParameter = value;
	GetPrivateProfileString( PLUGIN_NAME, _T("Extension"), _T("mp3"), value, MAX_PATH, inifile);
	g_strExtension = value;

	g_bDoTag = GetPrivateProfileInt( PLUGIN_NAME, _T("DoTag"), 1, inifile);
	g_bNoWAVHeader = GetPrivateProfileInt( PLUGIN_NAME, _T("NoWAVHeader"), 0, inifile);
	g_bShowConsole = GetPrivateProfileInt( PLUGIN_NAME, _T("ShowConsole"), 0, inifile);

	GetModuleFileName( hInstance, g_szEPFile, MAX_PATH);
	CString tmp = g_szEPFile;
	tmp.Format( _T("%s.xml"), tmp.Left( tmp.ReverseFind( '.')));
	GetPrivateProfileString( PLUGIN_NAME, _T("EPFile"), tmp, g_szEPFile, MAX_PATH, inifile);


	// init config dialog
	PluginPrefPage prefPage;
	prefPage.struct_size = sizeof(prefPage);
	prefPage.hModule = hInstance;
	prefPage.lpTemplate = MAKEINTRESOURCEW(IDD_PPP);
	prefPage.lpDisplayText = L"CLI Encoder";
	prefPage.lpDialogFunc = PPPDlgProc;
	prefPage.nCategory = PREFPAGE_CATEGORY_ENCODEFORMAT;
	prefPageID = QCDCallbacks.Service( opSetPluginPage, &prefPage, 0, 0);

	// return TRUE for successful initialization
	return TRUE;
}

//-----------------------------------------------------------------------------

void ShutDown(int flags)
{
	char inifile[MAX_PATH];
	char value[32];

	Complete(0); // force completing

	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	WritePrivateProfileString( PLUGIN_NAME, _T("Path"), g_strPath, inifile);
	WritePrivateProfileString( PLUGIN_NAME, _T("Parameter"), g_strParameter, inifile);
	WritePrivateProfileString( PLUGIN_NAME, _T("Extension"), g_strExtension, inifile);

	wsprintf(value, _T("%i"), g_bDoTag);
	WritePrivateProfileString( PLUGIN_NAME, _T("DoTag"), value, inifile);
	wsprintf(value, _T("%i"), g_bNoWAVHeader);
	WritePrivateProfileString( PLUGIN_NAME, _T("NoWAVHeader"), value, inifile);
	wsprintf(value, _T("%i"), g_bShowConsole);
	WritePrivateProfileString( PLUGIN_NAME, _T("ShowConsole"), value, inifile);

	WritePrivateProfileString( PLUGIN_NAME, _T("EPFile"), g_szEPFile, inifile);
}

//-----------------------------------------------------------------------------

BOOL Open(LPCSTR outFile, LPCSTR srcFile, WAVEFORMATEX *wf, LPSTR openedFilename, int openedFilenameSize)
{
	CString cmdline, tmp;
	BOOL temp_mode = FALSE;

	// destination file
	g_strDst = outFile;
	g_strDst += '.';
	g_strDst += g_strExtension;

	// source file
	g_strSrc = srcFile;

	// remember extension for tagging
	g_strExt = g_strExtension;

	// make command line
	tmp = g_strPath;
	tmp.Remove('\"');

	cmdline = '\"';
	cmdline += tmp + _T("\" ") + g_strParameter;

	// fix for .vbs script file
	if ( g_strPath.Mid( g_strPath.ReverseFind( '.'), 4) == _T(".vbs")) {
		tmp = _T("cscript.exe ");
		cmdline = tmp + cmdline;
	}

	// Start CLI Encoder
	g_cliEnc.Initialize(hwndParent, g_bNoWAVHeader, g_bShowConsole);
	if ( !g_cliEnc.Start(cmdline, g_strDst, wf))
		return FALSE;

	// copy filename of opened file back
	lstrcpy( openedFilename, g_strDst);


	// The following is recommended since encoders take a lot of CPU and 
	// this thread is generally set to a high priority by the input plugin.
	// Lowering the priority back to normal will make the player run smoother.

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL Write(WriteDataStruct* writeData)
{
	if ( !g_cliEnc.AddData( writeData->data, writeData->bytelen))
		return FALSE;


	// The following is recommended since encoders take a lot of CPU. 
	// This statement will help the player run smoother.

	Sleep(0);

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL Drain(int flags)
{
	if ( g_cliEnc.IsRunning())
		return FALSE;
	else
		return TRUE;
}

//-----------------------------------------------------------------------------

BOOL Complete(int flags)
{
    g_cliEnc.Stop();

	if ( flags && g_bDoTag) // tagging for normal complete only!
		TransferTag( g_strSrc, g_strDst, g_strExt);

	return TRUE;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	QCDCallbacks.Service( opShowPluginPage, hInstance, prefPageID, 0);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	CAboutDlg dlgAbout;

	dlgAbout.DoModal( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0));
}

//-----------------------------------------------------------------------------

INT_PTR CALLBACK PPPDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg)
	{
	case WM_INITDIALOG:
		{
			if ( g_pdlgParent == NULL) {
				g_pdlgParent = new CParentDlg;
				g_pdlgParent->Create( hwndDlg);
			}

			g_pdlgParent->ShowWindow( SW_SHOW);
		}

		return TRUE;
	case WM_PN_DIALOGSAVE:
		{
			if ( g_pdlgParent) {
				if ( g_pdlgParent->IsWindow()) {
					g_pdlgParent->SendMessage( WM_PN_DIALOGSAVE);
					g_pdlgParent->DestroyWindow();
				}

				delete g_pdlgParent; g_pdlgParent = NULL;
			}
		}

		return TRUE;
	}

	return FALSE;
}

