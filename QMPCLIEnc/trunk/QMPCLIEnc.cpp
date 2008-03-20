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
//	Copyright (C) 1997-2006 Quinnware
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
#include "QMPCLIEnc.h"

#include "ParentDlg.h"
#include "AboutDlg.h"

#include "IQCDMediaSource.h"

HINSTANCE		g_hInstance = NULL;
HWND			g_hwndParent;
QCDModInitEnc	QCDCallbacks;

WCHAR g_szPluginDisplayStr[1024] = {0};
WCHAR g_szPluginDisplayText[1024] = {0};

CString		g_strPath;
CString		g_strParameter;
CString		g_strExtension;
QCLIEncoder	g_cliEnc;
BOOL		g_bDoTag;
BOOL		g_bNoWAVHeader;
BOOL		g_bShowConsole;
CString		g_strEPFile;

int	prefPageID;
CString g_strSrc, g_strDst, g_strExt;

CParentDlg * g_pdlgParent = NULL;

INT_PTR CALLBACK PPPDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls( hInst);
		g_hInstance = hInst;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------

PLUGIN_API QCDModInitEnc* ENCODEDLL_ENTRY_POINT()
{
	QCDCallbacks.version = PLUGIN_API_VERSION_UNICODE;

	QCDCallbacks.toModule.Initialize		= Initialize;
	QCDCallbacks.toModule.ShutDown			= ShutDown;
	QCDCallbacks.toModule.Open				= Open;
	QCDCallbacks.toModule.Write				= Write;
	QCDCallbacks.toModule.Drain				= Drain;
	QCDCallbacks.toModule.Complete			= Complete;
	QCDCallbacks.toModule.Configure			= NULL;
	QCDCallbacks.toModule.About				= About;
	QCDCallbacks.toModule.TestFormat		= TestFormat;

	return &QCDCallbacks;
}

//-----------------------------------------------------------------------------

BOOL Initialize(QCDModInfo *modInfo, int flags)
{
	WCHAR inifile[MAX_PATH] = {0};

	modInfo->moduleString = (char*)g_szPluginDisplayStr;
	modInfo->moduleExtensions = (char*)g_szPluginDisplayText;

	g_hwndParent = (HWND)QCDCallbacks.Service( opGetParentWnd, 0, 0, 0);

	// load settings
	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	WCHAR value[MAX_PATH];
	GetPrivateProfileString( PLUGIN_NAME, _T("Path"), _T("LAME.EXE"), value, MAX_PATH, inifile);
	g_strPath = value;
	GetPrivateProfileString( PLUGIN_NAME, _T("Parameter"), _T("-V 2 --vbr-new - %d"), value, MAX_PATH, inifile);
	g_strParameter = value;
	GetPrivateProfileString( PLUGIN_NAME, _T("Extension"), _T("mp3"), value, MAX_PATH, inifile);
	g_strExtension = value;

	g_bDoTag = GetPrivateProfileInt( PLUGIN_NAME, _T("DoTag"), 1, inifile);
	g_bNoWAVHeader = GetPrivateProfileInt( PLUGIN_NAME, _T("NoWAVHeader"), 0, inifile);
	g_bShowConsole = GetPrivateProfileInt( PLUGIN_NAME, _T("ShowConsole"), 0, inifile);

	GetModuleFileName( g_hInstance, value, MAX_PATH);
	PathRemoveExtension( value); PathAddExtension( value, _T(".xml"));
	GetPrivateProfileString( PLUGIN_NAME, _T("EPFile"), value, value, MAX_PATH, inifile);
	g_strEPFile = value;

	// load display name
	ResInfo resInfo = { sizeof(ResInfo), g_hInstance, MAKEINTRESOURCE(IDS_DISPLAYNAME), 0, 0 };
	QCDCallbacks.Service( opLoadResString, (void*)g_szPluginDisplayStr, (long)sizeof(g_szPluginDisplayStr), (long)&resInfo);
	resInfo.resID = MAKEINTRESOURCE(IDS_DISPLAYEXT);
	QCDCallbacks.Service( opLoadResString, (void*)g_szPluginDisplayText, (long)sizeof(g_szPluginDisplayText), (long)&resInfo);


	// init config dialog
	PluginPrefPage prefPage;
	prefPage.struct_size = sizeof(PluginPrefPage);
	prefPage.hModule = g_hInstance;
	prefPage.lpTemplate = MAKEINTRESOURCEW(IDD_PPP);
	prefPage.lpDialogFunc = PPPDlgProc;
	prefPage.lpDisplayText = g_szPluginDisplayText;
	prefPage.nCategory = PREFPAGE_CATEGORY_ENCODEFORMAT;
	prefPage.hModuleParent = NULL;
	prefPage.groupID = 0;
	prefPage.createParam = 0;
	prefPage.hIcon = LoadIcon( g_hInstance, MAKEINTRESOURCEW(IDI_CLI));
	prefPageID = QCDCallbacks.Service( opSetPluginPage, &prefPage, 0, 0);
	DestroyIcon( prefPage.hIcon);


	// return TRUE for successful initialization
	return TRUE;
}

//-----------------------------------------------------------------------------

void ShutDown(int flags)
{
	WCHAR inifile[MAX_PATH];
	WCHAR value[32];

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

	WritePrivateProfileString( PLUGIN_NAME, _T("EPFile"), g_strEPFile, inifile);
}

//-----------------------------------------------------------------------------

int TestFormat(WAVEFORMATEX *wf, int flags)
{
	//
	// TODO : 
	// - verify format of audio data that will be received to make sure it's compatible
	//
	// Return one of the following
	// TESTFORMAT_ACCEPTED		// format compatible
	// TESTFORMAT_UNACCEPTED	// format not compatible
	// TESTFORMAT_FAILURE		// failure testing format
	// TESTFORMAT_NOTIMPL		// test not implemented

	return TESTFORMAT_NOTIMPL;
}

//-----------------------------------------------------------------------------

BOOL Open(LPCSTR outFile, LPCSTR srcFile, WAVEFORMATEX *wf, LPSTR openedFilename, int openedFilenameSize)
{
	CString cmdline, tmp;
	BOOL temp_mode = FALSE;

	// destination file
	g_strDst = (LPCWSTR)outFile;
	g_strDst += _T('.');
	g_strDst += g_strExtension;

	// source file
	g_strSrc = (LPCWSTR)srcFile;

	// remember extension for tagging
	g_strExt = g_strExtension;

	// make command line
	tmp = g_strPath;
	tmp.Remove( _T('\"'));

	cmdline = _T('\"');
	cmdline += tmp + _T("\" ") + g_strParameter;

	// fix for .vbs/.js script file
	tmp = g_strPath.Mid( g_strPath.ReverseFind( _T('.')), 4);
	if ( tmp == _T(".vbs") || tmp == _T(".js")) {
		tmp = _T("cscript.exe ");
		cmdline = tmp + cmdline;
	}

	// Start CLI Encoder
	g_cliEnc.Initialize(g_hwndParent, g_bNoWAVHeader, g_bShowConsole);
	if ( !g_cliEnc.Start(cmdline, g_strDst, wf))
		return FALSE;

	// copy filename of opened file back
	lstrcpyW( (LPWSTR)openedFilename, g_strDst);


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
	IQCDMediaInfo * piInfoSrc = NULL;
	IQCDTagInfo * piTagDst = NULL;
	BOOL ret = FALSE;

	g_cliEnc.Stop();

	if ( flags && g_bDoTag) { // trans-tag for normal complete only!

		// Get media source context
		piInfoSrc = (IQCDMediaInfo *)QCDCallbacks.Service( opGetIQCDMediaInfo, (void *)(LPCWSTR)g_strSrc, 0, 0);
		if ( piInfoSrc) {
			// Load MediaInfo from source
			long res = piInfoSrc->LoadFullData();
			if ( res) {
				// Get target tag context
				piTagDst = (IQCDTagInfo *)QCDCallbacks.Service( opGetIQCDTagInfo, (void *)(LPCWSTR)g_strDst, 0, 0);
				if ( piTagDst) {
					//Determine track #
					long idx = piInfoSrc->GetInfoIndexForMedia();

					// Set track tags from MediaInfo
					res = piTagDst->SetTagDataFromMediaInfo( piInfoSrc, idx, MEDIAINFO_ALWAYSSET);
					if ( res) {
						// write tag information into destination file
						res = piTagDst->WriteToFile( TAG_DEFAULT);
						if ( res)
							ret = TRUE;
					}
				}
			}
		}
	} else {
		// We did nothing
		return TRUE;
	}

	// Cleanup refs
	if ( piInfoSrc)
		piInfoSrc->Release();
	if ( piTagDst)
		piTagDst->Release();

	return ret;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	QCDCallbacks.Service( opShowPluginPage, g_hInstance, prefPageID, 0);
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

			if ( g_pdlgParent && g_pdlgParent->IsWindow())
				g_pdlgParent->ShowWindow( SW_SHOW);
		} return TRUE;

	case WM_PN_DIALOGSAVE:
		{
			if ( g_pdlgParent && g_pdlgParent->IsWindow())
				g_pdlgParent->SendMessage( WM_PN_DIALOGSAVE);
		} return TRUE;

	case WM_DESTROY:
		{
			if ( g_pdlgParent && g_pdlgParent->IsWindow()) {
				g_pdlgParent->DestroyWindow();
				delete g_pdlgParent; g_pdlgParent = NULL;
			}
		} return TRUE;
	}

	return FALSE;
}

