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

#include ".\qmpclienc.h"

#define PLUGIN_NAME "CLI Encoder"
#define PLUGIN_FULL_NAME "Commandline Encoder"
#define PLUGIN_VERSION "v1.0b1"

HINSTANCE		hInstance, hBrandInstance;
HWND			hwndParent;
QCDModInitEnc	QCDCallbacks;

QCLIEncoderPreset	g_cliEP;
QCLIEncoder			g_cliEnc;
ENCODER_PRESET		g_epCur;
BOOL				g_bNoWAVHeader;
BOOL				g_bShowConsole;
TCHAR				g_szEPFile[MAX_PATH];

int	prefPageID;

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
	QCDCallbacks.toModule.Configure			= Configure;
	QCDCallbacks.toModule.About				= About;

	return &QCDCallbacks;
}

//-----------------------------------------------------------------------------

BOOL Initialize(QCDModInfo *modInfo, int flags)
{
	char inifile[MAX_PATH];

	modInfo->moduleString = PLUGIN_FULL_NAME " " PLUGIN_VERSION;
	modInfo->moduleExtensions = PLUGIN_NAME;

	hwndParent = (HWND)QCDCallbacks.Service(opGetParentWnd, 0, 0, 0);

	// load settings
	QCDCallbacks.Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	char value[MAX_PATH];
	GetPrivateProfileString(PLUGIN_NAME, _T("Path"), _T("LAME.EXE"), value, MAX_PATH, inifile);
	g_epCur.m_strPath = value;
	GetPrivateProfileString(PLUGIN_NAME, _T("Parameter"), _T("--alt-preset standard - %d"), value, MAX_PATH, inifile);
	g_epCur.m_strParameter = value;
	GetPrivateProfileString(PLUGIN_NAME, _T("Extension"), _T("mp3"), value, MAX_PATH, inifile);
	g_epCur.m_strExtension = value;

	g_bNoWAVHeader = GetPrivateProfileInt(PLUGIN_NAME, _T("NoWAVHeader"), 0, inifile);
	g_bShowConsole = GetPrivateProfileInt(PLUGIN_NAME, _T("ShowConsole"), 0, inifile);

	GetModuleFileName( hInstance, g_szEPFile, MAX_PATH);
	CString tmp = g_szEPFile;
	tmp.Format( _T("%s.ep"), tmp.Left( tmp.ReverseFind( '.')));
	GetPrivateProfileString(PLUGIN_NAME, _T("EPFile"), tmp, g_szEPFile, MAX_PATH, inifile);


	// init config dialog
	PluginPrefPage prefPage;
	prefPage.struct_size = sizeof(prefPage);
	prefPage.hModule = hInstance;
	prefPage.lpTemplate = MAKEINTRESOURCEW(IDD_PARENT);
	prefPage.lpDisplayText = L"CLI Encoder";
	prefPage.lpDialogFunc = ParentDlgProc;
	prefPage.nCategory = PREFPAGE_CATEGORY_ENCODEFORMAT;
	prefPageID = QCDCallbacks.Service(opSetPluginPage, &prefPage, 0, 0);

	// return TRUE for successful initialization
	return TRUE;
}

//-----------------------------------------------------------------------------

void ShutDown(int flags)
{
	char inifile[MAX_PATH];
	char value[32];

	Complete(0); // forece completing

	QCDCallbacks.Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	WritePrivateProfileString(PLUGIN_NAME, _T("Path"), g_epCur.m_strPath, inifile);
	WritePrivateProfileString(PLUGIN_NAME, _T("Parameter"), g_epCur.m_strParameter, inifile);
	WritePrivateProfileString(PLUGIN_NAME, _T("Extension"), g_epCur.m_strExtension, inifile);

	wsprintf(value, _T("%i"), g_bNoWAVHeader);
	WritePrivateProfileString(PLUGIN_NAME, _T("NoWAVHeader"), value, inifile);
	wsprintf(value, _T("%i"), g_bShowConsole);
	WritePrivateProfileString(PLUGIN_NAME, _T("ShowConsole"), value, inifile);

	WritePrivateProfileString(PLUGIN_NAME, _T("EPFile"), g_szEPFile, inifile);
}

//-----------------------------------------------------------------------------

BOOL Open(LPCSTR outFile, LPCSTR srcFile, WAVEFORMATEX *wf, LPSTR openedFilename, int openedFilenameSize)
{
	CString cmdline, src, dst, tmp;
	BOOL temp_mode = FALSE;

	// det-file
	dst = outFile;
	dst += '.';
	dst += g_epCur.m_strExtension;

	// src-file
	src = srcFile;

	// make commandline
	tmp = g_epCur.m_strPath;
	tmp.Trim('\"');

	cmdline = '\"';
	cmdline += tmp + _T("\" ") + g_epCur.m_strParameter;

	// fix for .vbs script file
	if ( g_epCur.m_strPath.Mid( g_epCur.m_strPath.ReverseFind( '.'), 4) == _T(".vbs")) {
		tmp = _T("cscript.exe ");
		cmdline = tmp + cmdline;
	}

	// Start CLI Encoder
	if ( !g_cliEnc.Initialize(hwndParent, g_bNoWAVHeader, g_bShowConsole) || 
        !g_cliEnc.Start(cmdline, dst, wf))
		return FALSE;

	// copy filename of opened file back
	lstrcpy( openedFilename, dst);


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

	return TRUE;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	QCDCallbacks.Service(opShowPluginPage, hInstance, prefPageID, 0);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	DialogBoxIndirect(hInstance, 
		LoadResDialog(IDD_ABOUT), 
		(HWND)QCDCallbacks.Service(opGetPropertiesWnd, NULL, 0, 0), 
		(DLGPROC)AboutDlgProc);
}

//-----------------------------------------------------------------------------

