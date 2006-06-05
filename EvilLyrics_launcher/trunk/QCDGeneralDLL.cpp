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

#define PLUGIN_NAME			"EvilLyrics Launcher v2.0"
#define IDM_LAUNCH			65001
//#define ID_HOTKEY			65002

#define VC_EXTRALEAN		1		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <TCHAR.H>
#include "..\..\Includes\Win32Error.h"
#include "..\QCD_Includes\QCDAccel.h"
#include "..\..\Includes\Homemade\Inifile.h"
#include "QCDGeneralDLL.h"

HINSTANCE		hInstance;
HWND			hwndPlayer;
QCDModInitGen	*QCDCallbacks;

_TCHAR*			g_strFile;
_TCHAR*			g_strDirectory;
AccelInfo		g_Hotkey;
BOOL			g_bInsertMenuItem;
BOOL			g_bAutoStart;
BOOL			g_bAutoClose;
// BOOL			g_bMinimizeOnStart;

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

PLUGIN_API BOOL GENERALDLL_ENTRY_POINT(QCDModInitGen *ModInit, QCDModInfo *ModInfo)
{
	ModInit->version = PLUGIN_API_VERSION;
	ModInfo->moduleString = PLUGIN_NAME;

	ModInit->toModule.ShutDown			= ShutDown;
	ModInit->toModule.About				= About;
	ModInit->toModule.Configure			= Configure;

	QCDCallbacks = ModInit;

	hwndPlayer = (HWND)QCDCallbacks->Service(opGetParentWnd, 0, 0, 0);

	// TODO: all your plugin initialization here

	// Subclass the player and listen for WM_PN_?
	/*if ((QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDSubProc)) == 0) {
		MessageBox(hwndPlayer, _T("Failed to subclass player!"), _T("Fatal init error!"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}*/

	g_strFile			= NULL;
	g_strDirectory		= NULL;

	// Get filename and directory
	char inifile[MAX_PATH];
	int iSize = 0;
	_TCHAR* strTemp = new _TCHAR[_MAX_PATH];

	QCDCallbacks->Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);
	CIniFile oIni(inifile, _T("EvilLyricsLauncher"));	

	g_bAutoStart		= oIni.GetBool(_T("AutoStart"), FALSE);
	g_bAutoClose		= oIni.GetBool(_T("AutoClose"), TRUE);
	g_Hotkey.key		= oIni.GetUInt(_T("HotkeyKey"), 0);
	g_Hotkey.modifiers	= oIni.GetUInt(_T("HotkeyMod"), 0);
	g_bInsertMenuItem	= oIni.GetBool(_T("InsertMenuItem"), TRUE);
	// g_bMinimizeOnStart	= oIni.GetBool(_T("MinimizeOnStart"), TRUE);

	// Filename
	oIni.GetString(_T("Filename"), strTemp, _T(""), _MAX_PATH);
	iSize = _tcslen(strTemp);
	if (iSize > 1) {
		g_strFile = new _TCHAR[iSize + 1];
		_tcsncpy(g_strFile, strTemp, iSize + 1);
	}
	else {
		Configure(0);
		return TRUE;	// Dont run rest of init
	}
	// Path
	oIni.GetString(_T("Directory"), strTemp, _T(""), _MAX_PATH);
	iSize = _tcslen(strTemp);
	if (iSize > 1) {
		g_strDirectory = new _TCHAR[iSize + 1];
		_tcsncpy(g_strDirectory, strTemp, iSize + 1);
	}
	else
		Configure(0);
	
	if (g_bInsertMenuItem)
		MakeMenu();
	if (g_bAutoStart)
		StartEvilLyrics();
	if (g_bAutoClose)
		CloseEvilLyrics();


	// Clean up
	delete [] strTemp;
	
	return TRUE;	// return TRUE for successful initialization
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	// QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDProc);

	if (g_bAutoClose)
		CloseEvilLyrics();

	// Save to ini file
	char inifile[MAX_PATH];
	QCDCallbacks->Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);
	CIniFile oIni(inifile, _T("EvilLyricsLauncher"));

	oIni.SetBool(_T("AutoStart"), g_bAutoStart);
	oIni.SetBool(_T("AutoClose"), g_bAutoClose);
	oIni.SetBool(_T("InsertMenuItem"), g_bInsertMenuItem);
	
	oIni.SetUInt(_T("HotkeyKey"), g_Hotkey.key);
	oIni.SetUInt(_T("HotkeyMod"), g_Hotkey.modifiers);
	

	if (g_strFile) {
		oIni.SetString(_T("Filename"), g_strFile);
		delete [] g_strFile;
	}
	if (g_strDirectory) {
		oIni.SetString(_T("Directory"), g_strDirectory);
		delete [] g_strDirectory;
	}

	oIni.Flush();		
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	switch (flags)
	{
	case IDM_LAUNCH : 
	{
		StartEvilLyrics();

		break;
	}
	default :
	{
		// Get the preference dialog hwnd
		HWND hwndParent = (HWND)QCDCallbacks->Service(opGetPropertiesWnd, NULL, 0, 0);
		if (!hwndParent)
			hwndParent = hwndPlayer;
		
		
		HWND dlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hwndParent, (DLGPROC)ConfigDialogCallback);
		if (dlg) {
			ShowWindow(dlg, SW_SHOWNORMAL);
			if (!g_strDirectory || !g_strFile)
				SendMessage(dlg, WM_COMMAND, MAKEWPARAM(IDC_BROWSE, BN_CLICKED), 0);

		}
		else {
			CWin32Error e;
			MessageBox(hwndPlayer, e, _T("Error"), MB_OK);
		}
	}
	}	// switch
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	MessageBox(hwndPlayer, _T("Plug-in to support\nthe EvilLyrics program.\n\n" 
		"Plug-in by:\nToke Noer\ntoke@noer.it"), _T("About"), MB_OK | MB_ICONINFORMATION);
}

//-----------------------------------------------------------------------------

void StartEvilLyrics()
{
	if (g_strFile && g_strDirectory && !FindEvilLyrics()) {
		INT iError = (UINT)ShellExecute(NULL, NULL, g_strFile, NULL, g_strDirectory, SW_SHOWNORMAL);

		if (iError <= 32) {
			CWin32Error e;
			MessageBox(hwndPlayer, e, _T("Error"), MB_OK);
		}
		
		/*if (g_bMinimizeOnStart) {
			// Max wait 1 sec for EL to start
			for (int i = 0; i < 100; i++)
			{
				Sleep(10);	// Dont sleep to long... it will lock QCD interface

				HWND hEL = FindEvilLyrics();
				if (hEL) {
					PostMessage(hEL, WM_SYSCOMMAND, SC_MINIMIZE, 0);
					break;
				}
			}
		}*/
	}
}

//-----------------------------------------------------------------------------

void CloseEvilLyrics()
{
	HWND hEL = FindEvilLyrics();
	if (hEL)
		SendMessage(hEL, WM_CLOSE, 0, 0);
}

//-----------------------------------------------------------------------------

HWND FindEvilLyrics()
{
	HWND hEL = FindWindow(NULL, _T("EvilLyrics"));
	if (hEL)
		return hEL;
	else
		return NULL;
}

//-----------------------------------------------------------------------------

void MakeMenu()
{
	_TCHAR strMenu[256];
	_TCHAR strAccel[16];
	_tcsncpy(strMenu, _T("Launch EvilLyrics"), 255);

	if (g_Hotkey.key) {
		QCDCallbacks->Service(opSetAccelerator, hInstance, IDM_LAUNCH, (LONG)&g_Hotkey);

		TranslateAccelToText(g_Hotkey.key, g_Hotkey.modifiers, strAccel);
		_tcsncat(strMenu, _T("\t"), 4);
		_tcsncat(strMenu, strAccel, 16);
	}

	QCDCallbacks->Service(opSetMainMenuItem, hInstance, IDM_LAUNCH, (long)strMenu);
}

//-----------------------------------------------------------------------------

void BrowseForFile()
{
	OPENFILENAME ofn;
	_TCHAR strPath[_MAX_PATH];

	ZeroMemory(strPath, _MAX_PATH);
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize		= sizeof(ofn);
	ofn.hwndOwner		= hwndPlayer;
	ofn.lpstrFilter		= "EvilLyrics.exe\0EvilLyrics.exe\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrTitle		= _T("Specify location of EvilLyrics.exe");
	ofn.Flags			= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrFile		= strPath;
	ofn.nMaxFile		= sizeof(strPath);

	delete [] g_strFile;
	delete [] g_strDirectory;

	if (GetOpenFileName(&ofn)) {
		g_strFile = new _TCHAR[_tcslen(ofn.lpstrFile) - ofn.nFileOffset + sizeof(_TCHAR) + 1];
		g_strDirectory = new _TCHAR[_tcslen(ofn.lpstrFile) + sizeof(_TCHAR)];
			
		ZeroMemory(g_strFile, _tcslen(ofn.lpstrFile) - ofn.nFileOffset + sizeof(_TCHAR));
		ZeroMemory(g_strDirectory, _tcslen(ofn.lpstrFile) + sizeof(_TCHAR));

		_tcsncpy(g_strDirectory, strPath, ofn.nFileOffset - 1);
		_tcscpy(g_strFile, strPath + ofn.nFileOffset);

		// Save to INI
		char inifile[MAX_PATH];
		QCDCallbacks->Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);
		CIniFile oIni(inifile, _T("EvilLyricsLauncher"));
		oIni.SetString(_T("Filename"), g_strFile);
		oIni.SetString(_T("Directory"), g_strDirectory);
		oIni.Flush();
	}
	else {
		g_strFile		= NULL;
		g_strDirectory	= NULL;

		DWORD iExtErr = CommDlgExtendedError();
		if (iExtErr > 0) {	// Not sure this will work
			CWin32Error e;
			MessageBox(hwndPlayer, e, _T("Error"), MB_OK);
		}
	}
}

//-----------------------------------------------------------------------------

BOOL CALLBACK ConfigDialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG : {
		CheckDlgButton(hwndDlg, IDC_AUTOSTART, g_bAutoStart);
		CheckDlgButton(hwndDlg, IDC_AUTOCLOSE, g_bAutoClose);
		CheckDlgButton(hwndDlg, IDC_MENUITEM, g_bInsertMenuItem);

		// Set path
		_TCHAR strLocation[_MAX_PATH];
		if (g_strDirectory && g_strFile) {
			_tcsncpy(strLocation, g_strDirectory, _MAX_PATH);
			_tcsncat(strLocation, _T("\\"), _MAX_PATH);
			_tcsncat(strLocation, g_strFile, _MAX_PATH);
		}
		else
			_tcsncpy(strLocation, _T("None"), _MAX_PATH);
		SetDlgItemText(hwndDlg, IDC_LOCATION, strLocation);

		// Hotkey
		HWND hHotkeyCtrl = GetDlgItem(hwndDlg, IDC_HOTKEY);
		if (hHotkeyCtrl)
			SendMessage(hHotkeyCtrl, HKM_SETHOTKEY, MAKEWORD(g_Hotkey.key, TranslateAccelToNative(g_Hotkey.modifiers)), 0);
		else
			MessageBox(hwndDlg, _T("Fail on: GetDlgItem(hwndDlg, IDC_HOTKEY);"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);

		// return TRUE;		// In response to a WM_INITDIALOG message, the dialog box procedure should return zero if it calls the SetFocus function to set the focus to one of the controls in the dialog box. Otherwise, it should return nonzero, in which case the system sets the focus to the first control in the dialog box that can be given the focus.
		break;
	}

	case WM_CLOSE :
		DestroyWindow(hwndDlg);
		return TRUE;

	case WM_COMMAND : 
		switch (LOWORD(wParam))
		{
		case IDOK : {
			// Save variables
			g_bAutoStart		= IsDlgButtonChecked(hwndDlg, IDC_AUTOSTART);
			g_bAutoClose		= IsDlgButtonChecked(hwndDlg, IDC_AUTOCLOSE);
			g_bInsertMenuItem	= IsDlgButtonChecked(hwndDlg, IDC_MENUITEM);

			if (g_bInsertMenuItem)
				MakeMenu();
			else
				QCDCallbacks->Service(opSetMainMenuItem, hInstance, IDM_LAUNCH, 0);

			// Hotkey
			HWND hHotkeyCtrl = GetDlgItem(hwndDlg, IDC_HOTKEY);
			if (hHotkeyCtrl) {
				LRESULT nResult = SendMessage(hHotkeyCtrl, HKM_GETHOTKEY, 0, 0);
				g_Hotkey.key = LOBYTE(nResult);
				g_Hotkey.modifiers = TranslateAccelToQCD(HIBYTE(nResult));
			}
			else
				MessageBox(hwndDlg, _T("Fail on: GetDlgItem(hwndDlg, IDC_HOTKEY);"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);


			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;
		}
		case IDCANCEL :
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;

		case IDC_BROWSE :
			BrowseForFile();
			// Set path
			_TCHAR strLocation[_MAX_PATH];
			if (g_strDirectory && g_strFile) {
				_tcsncpy(strLocation, g_strDirectory, _MAX_PATH);
				_tcsncat(strLocation, _T("\\"), _MAX_PATH);
				_tcsncat(strLocation, g_strFile, _MAX_PATH);
			}
			else
				_tcsncpy(strLocation, _T("None"), _MAX_PATH);

			SetDlgItemText(hwndDlg, IDC_LOCATION, strLocation);

			break;
			
		} // switch (LOWORD(wParam))

		break;

	} // Switch (uMsg)
	
	return FALSE;	// return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}


/*LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{ 
	/*switch (msg)
	{
		/*case WM_COMMAND :
		{
			if (HIWORD(wparam) == 0) {
				if (LOWORD(wparam) == IDM_LAUNCH) { // LOWORD(wparam) == wID
					INT iError = (UINT)ShellExecute(NULL, NULL, g_strFile, NULL, g_strDirectory, SW_SHOWDEFAULT);

					if (iError <= 32) {
						CWin32Error e;
						MessageBox(hwndPlayer, e, _T("Error"), MB_OK);
					}					
				}
			}
			break;
		}*/

		/*case WM_INITMENUPOPUP :
		{
			static BOOL  bMainMenu = FALSE;
			static HMENU hMainMenu = NULL;
			BOOL bTrayMenu;
			
			if (IsMainMenu((HMENU)wparam, &bTrayMenu)) {
				bMainMenu = TRUE;
				hMainMenu = (HMENU)wparam;

				// Menu item
				_TCHAR* strMenuName = _T("Launch EvilLyrics");
				MENUITEMINFO miiMenu, miiSeparator;
				miiMenu.cbSize		= sizeof(MENUITEMINFO);
				miiMenu.fMask		= MIIM_TYPE | MIIM_ID;
				miiMenu.fType		= MFT_STRING;
				miiMenu.fState		= MFS_ENABLED;
				miiMenu.wID			= IDM_EVILLYRICS;
				miiMenu.dwItemData	= 500;
				miiMenu.dwTypeData	= strMenuName;
				miiMenu.cch			= _tcslen(strMenuName);

				// Separator
				miiSeparator.cbSize		= sizeof(MENUITEMINFO);
				miiSeparator.fMask		= MIIM_TYPE | MIIM_ID;
				miiSeparator.fType		= MFT_SEPARATOR;
				miiSeparator.fState		= MFS_ENABLED;

				if (bTrayMenu) {
					InsertMenuItem((HMENU)wparam, 40004, FALSE, &miiMenu);
					InsertMenuItem((HMENU)wparam, 40004, FALSE, &miiSeparator);	// TRAY_PREFERENCES
				}
				else {
					InsertMenuItem((HMENU)wparam, 40004, FALSE, &miiMenu);
					InsertMenuItem((HMENU)wparam, 40004, FALSE, &miiSeparator);	// TRAY_PREFERENCES
				}
					
			}
			break;
		} // Case
	}
	
	return CallWindowProc(QCDProc, hwnd, msg, wparam, lparam);
}*/