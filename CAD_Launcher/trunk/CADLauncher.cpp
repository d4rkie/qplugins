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
//-----------------------------------------------------------------------------
// Plug-in written by Toke Noer : toke@noer.it
//-----------------------------------------------------------------------------
// Version history:
//-----------------------------------------------------------------------------
// Todo:
//-----------------------------------------------------------------------------

#define VC_EXTRALEAN		1		// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN 1

#include <Windows.H>
#include <shellapi.h>
#include <STDLIB.H>
#include <TCHAR.H>
#include "..\..\Includes\Win32Error.h" // See http://www.codeproject.com/win32/cwin32error.asp
#include "Config.h"
#include "CADLauncher.h"

//-----------------------------------------------------------------------------
static const CHAR*  PLUGIN_NAME_A  =  "CAD Launcher v1.1";
static const WCHAR* PLUGIN_NAME_W  = L"CAD Launcher v1.1";
static const WCHAR* INI_SECTION    = L"CADLauncher";

static const WCHAR* MSG_STANDARD  = _T("CD Art Display Launcher");
static const WCHAR* MSG_ERROR     = _T("CADLauncher error");
//-----------------------------------------------------------------------------
static const LONG   IDM_MENUITEM1 = 50234;
//-----------------------------------------------------------------------------
HINSTANCE		hInstance;
HWND			hwndPlayer;
QCDModInitGen	*QCDCallbacks;
Settings         settings;
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
	ModInit->version					= PLUGIN_API_VERSION_WANTUTF8;
	ModInfo->moduleString				= (char*)PLUGIN_NAME_A;

	ModInit->toModule.ShutDown			= ShutDown;
	ModInit->toModule.About				= About;
	ModInit->toModule.Configure			= Configure;

	QCDCallbacks = ModInit;

	hwndPlayer = (HWND)QCDCallbacks->Service(opGetParentWnd, 0, 0, 0);

	LoadSettings();

	if (settings.bStartCad)
		StartCad();

	// Insert menues
	QCDCallbacks->Service(opSetPluginMenuItem, (void*)hInstance, IDM_MENUITEM1, (long)"Launch CD Art Display");
	
	return TRUE; // return TRUE for successful initialization
}

//----------------------------------------------------------------------------

void ShutDown(int flags)
{
	QCDCallbacks->Service(opSetPluginMenuItem, (void*)hInstance, 0, 0);

	if (settings.bCloseCad)
		CloseCad();

	SaveSettings();
	FreeCadPath();
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	if (flags == IDM_MENUITEM1)
		StartCad();
	else
	{
		HWND hwnd = (HWND)QCDCallbacks->Service(opGetPropertiesWnd, NULL, 0, 0);
		if (!hwnd)
			hwnd = hwndPlayer;
		CConfig dlg(hInstance, hwnd);
	}
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	_TCHAR str[256];
	_tcscpy_s(str, 256, PLUGIN_NAME_W);
	_tcscat_s(str, 256, _T("\n\nPlug-in by:\nToke Noer (toke@noer.it)"));
	MessageBox(hwndPlayer, str, _T("About"), MB_OK | MB_ICONINFORMATION);
}

//-----------------------------------------------------------------------------

void LoadSettings()
{
	settings.strCadFile = NULL;
	settings.strCadPath = NULL;

	WCHAR strIni[MAX_PATH];
	WCHAR strTmp[MAX_PATH];
	QCDCallbacks->Service(opGetPluginSettingsFile, strTmp, MAX_PATH*sizeof(WCHAR), 0); // Returns UTF8
	QCDCallbacks->Service(opUTF8toUCS2, strTmp, (long)strIni, MAX_PATH);

	settings.bFirstRun = GetPrivateProfileIntW(INI_SECTION, L"bFirstRun", 1, strIni);
	settings.bStartCad = GetPrivateProfileIntW(INI_SECTION, L"bStartCad", 1, strIni);
	settings.bCloseCad = GetPrivateProfileIntW(INI_SECTION, L"bCloseCad", 1, strIni);

	if (settings.bFirstRun)
	{
		settings.bFirstRun = false;

		MessageBox(hwndPlayer, L"This is the first time you run the CD Art Display launcher plug-in.\n\n" \
			L"The plug-in will now try to find the CD Art Display application...", MSG_STANDARD, 0);
		if (GetCadFilePath())
			MessageBox(hwndPlayer, L"CAD.exe found.", MSG_STANDARD, 0);
		else
			MessageBox(hwndPlayer, L"CAD.exe was not found.\n\nPlease go to the plug-in configuration and browse for CAD.exe!", MSG_STANDARD, 0);
	}
	else
	{
		GetPrivateProfileStringW(INI_SECTION, L"CadPath", L"", strTmp, MAX_PATH, strIni);
		if (wcslen(strTmp) > 3) {
			settings.strCadPath = new WCHAR[wcslen(strTmp) + 1];
			wcscpy_s(settings.strCadPath, wcslen(strTmp) + 1, strTmp);
		}

		GetPrivateProfileStringW(INI_SECTION, L"CadFile", L"", strTmp, MAX_PATH, strIni);
		if (wcslen(strTmp) > 1) {
			settings.strCadFile = new WCHAR[wcslen(strTmp) + 1];
			wcscpy_s(settings.strCadFile, wcslen(strTmp) + 1, strTmp);
		}
	}
}

//-----------------------------------------------------------------------------

void SaveSettings()
{
	WCHAR buf[32];
	WCHAR strIni[MAX_PATH];
	WCHAR strTmp[MAX_PATH];
	QCDCallbacks->Service(opGetPluginSettingsFile, strTmp, MAX_PATH*sizeof(WCHAR), 0); // Returns UTF8
	QCDCallbacks->Service(opUTF8toUCS2, strTmp, (long)strIni, MAX_PATH);
	
	wsprintfW(buf, L"%u", settings.bFirstRun);  WritePrivateProfileStringW(INI_SECTION, L"bFirstRun", buf, strIni);
	wsprintfW(buf, L"%u", settings.bStartCad);  WritePrivateProfileStringW(INI_SECTION, L"bStartCad", buf, strIni);
	wsprintfW(buf, L"%u", settings.bCloseCad);  WritePrivateProfileStringW(INI_SECTION, L"bCloseCad", buf, strIni);

	WritePrivateProfileStringW(INI_SECTION, L"CadFile", settings.strCadFile, strIni);
	WritePrivateProfileStringW(INI_SECTION, L"CadPath", settings.strCadPath, strIni);
}

//-----------------------------------------------------------------------------

void StartCad()
{
	if (settings.strCadFile && settings.strCadPath && !FindCadWindow())
	{
		INT iError = (UINT)ShellExecute(NULL, NULL, settings.strCadFile, NULL, settings.strCadPath, SW_SHOWNORMAL);

		if (iError <= 32)
			MessageBox(hwndPlayer, CWin32Error(), MSG_ERROR, MB_OK);
	}
}

//-----------------------------------------------------------------------------

void CloseCad()
{
	HWND hWnd = FindCadWindow();
	if (hWnd)
		SendMessage(hWnd, WM_CLOSE, 0, 0);
}

//-----------------------------------------------------------------------------

HWND FindCadWindow()
{
	HWND hWnd = FindWindow(NULL, _T("CD Art Display"));
	if (hWnd)
		return hWnd;
	else
		return NULL;
}

//-----------------------------------------------------------------------------

BOOL GetCadFilePath()
{
	static const WCHAR* REGKEY = L"Software\\CD Art Display";
	HKEY hKey = NULL;

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY, 0,  KEY_READ, &hKey))
	{
		CHAR strBuffer[MAX_PATH*sizeof(WCHAR)] = {0};
		DWORD nBufferSize = MAX_PATH;
		DWORD nType = 0;

		if (ERROR_SUCCESS != RegQueryValueExW(hKey, NULL, NULL, &nType, (LPBYTE)strBuffer, &nBufferSize)) {
			CWin32Error e;
			MessageBox(hwndPlayer, e, MSG_ERROR, MB_OK);			
			RegCloseKey(hKey);
			return FALSE;
		}
		RegCloseKey(hKey);

		if (nBufferSize == MAX_PATH*sizeof(WCHAR)) // Might not be 0 terminated
			strBuffer[MAX_PATH*sizeof(WCHAR) - 1] = 0;

		if (nBufferSize > 0)
			ParseCadPath((LPCWSTR)strBuffer);

		return TRUE;
	}
	else
		return FALSE;
}

//-----------------------------------------------------------------------------

inline void FreeCadPath()
{
	delete [] settings.strCadPath; settings.strCadPath = NULL;
	delete [] settings.strCadFile; settings.strCadFile = NULL;
}

//-----------------------------------------------------------------------------

void ParseCadPath(LPCWSTR str)
{
	WCHAR strDrive[_MAX_DRIVE], strDir[_MAX_DIR], strFileName[_MAX_FNAME], strExt[_MAX_EXT];

	if (0 == _wsplitpath_s((LPCWSTR)str, strDrive, _MAX_DRIVE, strDir, _MAX_DIR, strFileName, _MAX_FNAME, strExt, _MAX_EXT))
	{ // success
		UINT nPathSize = 0, nFileSize = 0;
		nPathSize = wcslen(strDrive) + wcslen(strDir) + 1;
		nFileSize = wcslen(strFileName) + wcslen(strExt) + 1;

		FreeCadPath();
		settings.strCadPath = new WCHAR[nPathSize];
		settings.strCadFile = new WCHAR[nFileSize];

		if (settings.strCadPath == NULL || settings.strCadFile == NULL) {
			FreeCadPath();
			return;
		}

		wcscpy_s(settings.strCadPath, nPathSize, strDrive);
		wcscat_s(settings.strCadPath, nPathSize, strDir);

		wcscpy_s(settings.strCadFile, nFileSize, strFileName);
		wcscat_s(settings.strCadFile, nFileSize, strExt);
	}
}

//-----------------------------------------------------------------------------