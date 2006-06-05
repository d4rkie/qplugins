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
// Version history:
//-----------------------------------------------------------------------------
// Todo:
//   
//-----------------------------------------------------------------------------

#define PLUGIN_NAME _T("Snarl v1.0")

#include <TCHAR.H>
//#include <stdio.h>

#include "Snarl.h"
#include "Config.h"
#include "QCDGeneralDLL.h"

HINSTANCE        hInstance;
HWND             hwndPlayer;
QCDModInitGen    *QCDCallbacks;
WNDPROC          QCDProc;

char*            g_strIcon;
int		         g_nMsgId;
Settings         settings;

static const WCHAR INI_SECTION[] = L"Snarl";
static const int WM_REGISTER_MSG = 61091;

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
	ModInit->version				= PLUGIN_API_WANTUTF8;
	ModInfo->moduleString			= PLUGIN_NAME;

	ModInit->toModule.ShutDown		= ShutDown;
	ModInit->toModule.About			= About;
	ModInit->toModule.Configure		= Configure;

	QCDCallbacks = ModInit;

	hwndPlayer = (HWND)QCDCallbacks->Service(opGetParentWnd, 0, 0, 0);

	// Subclass the player and listen for WM_PN_?
	if ((QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDSubProc)) == 0) {
		MessageBox(hwndPlayer, _T("Failed to subclass player!"), _T("Fatal init error!"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	LoadSettings();
	snRegisterConfig(hwndPlayer, "Quintessential Media Player", WM_REGISTER_MSG);

	// Display if IsPlaying()
	if (IsPlaying())
		DisplaySongInfo();

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDProc);
	snRevokeConfig(hwndPlayer);
	delete [] g_strIcon;
	SaveSettings();
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	HWND hwnd = (HWND)QCDCallbacks->Service(opGetPropertiesWnd, NULL, 0, 0);
	if (!hwnd)
		hwnd = hwndPlayer;
	CConfig dlg(hInstance, hwnd);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	MessageBox(hwndPlayer, PLUGIN_NAME _T("\n\nPlug-in by:\nToke Noer (toke@noer.it)"), _T("About"), MB_OK | MB_ICONINFORMATION);
}

//-----------------------------------------------------------------------------

void LoadSettings()
{
	WCHAR strIni[MAX_PATH];
	WCHAR strTmp[MAX_PATH];
	QCDCallbacks->Service(opGetPluginSettingsFile, strTmp, MAX_PATH*sizeof(WCHAR), 0); // Returns UTF8
	QCDCallbacks->Service(opUTF8toUCS2, strTmp, (long)strIni, MAX_PATH);

	settings.nTimeout           = GetPrivateProfileIntW(INI_SECTION, L"nTimeout", 7, strIni);
	settings.bCascade           = GetPrivateProfileIntW(INI_SECTION, L"bCascade", 0, strIni);
	settings.bHeadline_wrap      = GetPrivateProfileIntW(INI_SECTION, L"bHeadline_Wrap", 0, strIni);
	//settings.bText1_wrap         = GetPrivateProfileIntW(INI_SECTION, L"bText1_Wrap", 1, strIni);
	//settings.bText2_wrap         = GetPrivateProfileIntW(INI_SECTION, L"bText2_Wrap", 0, strIni);
	settings.Headline_ServiceOp = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bHeadline_Op", opGetArtistName, strIni);
	settings.Text1_ServiceOp    = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bText1_Op", opGetTrackName, strIni);
	settings.Text2_ServiceOp    = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bText2_Op", opGetDiscName, strIni);

	// Get icon path
	char strFolder[MAX_PATH];
	QCDCallbacks->Service(opGetPluginFolder, strFolder, MAX_PATH, 0);
	int nStrPathLen = strlen(strFolder);
	nStrPathLen += 11; // "\\snarl.png" + null
	g_strIcon = new char[nStrPathLen];
	strcpy_s(g_strIcon, nStrPathLen, strFolder);
	strcat_s(g_strIcon, nStrPathLen, "\\snarl.png");
}

//-----------------------------------------------------------------------------

void SaveSettings()
{
	WCHAR buf[32];
	WCHAR strIni[MAX_PATH];
	WCHAR strTmp[MAX_PATH];
	QCDCallbacks->Service(opGetPluginSettingsFile, strTmp, MAX_PATH*sizeof(WCHAR), 0); // Returns UTF8
	QCDCallbacks->Service(opUTF8toUCS2, strTmp, (long)strIni, MAX_PATH);
	
	wsprintfW(buf, L"%u", settings.nTimeout);          	WritePrivateProfileStringW(INI_SECTION, L"nTimeout",        buf, strIni);
	wsprintfW(buf, L"%u", settings.bCascade);          	WritePrivateProfileStringW(INI_SECTION, L"bCascade",        buf, strIni);
	wsprintfW(buf, L"%u", settings.bHeadline_wrap);     WritePrivateProfileStringW(INI_SECTION, L"bHeadline_Wrap",  buf, strIni);
	//wsprintfW(buf, L"%u", settings.bText1_wrap);        WritePrivateProfileStringW(INI_SECTION, L"bText1_Wrap",     buf, strIni);
	//wsprintfW(buf, L"%u", settings.bText2_wrap);        WritePrivateProfileStringW(INI_SECTION, L"bText2_Wrap",     buf, strIni);
	wsprintfW(buf, L"%u", settings.Headline_ServiceOp); WritePrivateProfileStringW(INI_SECTION, L"bHeadline_Op",    buf, strIni);
	wsprintfW(buf, L"%u", settings.Text1_ServiceOp);    WritePrivateProfileStringW(INI_SECTION, L"bText1_Op",       buf, strIni);
	wsprintfW(buf, L"%u", settings.Text2_ServiceOp);    WritePrivateProfileStringW(INI_SECTION, L"bText2_Op",       buf, strIni);
}

//-----------------------------------------------------------------------------

int FindSpace(char* str, int nPos)
{
	if (!str || !*str)
		return -1;

	while (nPos > 0)
	{
		if (str[nPos] == ' ')
			return nPos;
		nPos--;
	}
	return -1;
}

//-----------------------------------------------------------------------------

void InsertNextLine(char* str, int nPos)
{
	char strTemp[1024];
	int strLen = strlen(str);
	int nSpace = FindSpace(str, nPos); // Find space before edge
	
	/*char strD[1024];
	sprintf(strD, "nSpace: %d", nSpace);
	MessageBox(hwndPlayer, strD, "Debug", 0);*/

	if (nSpace > 5 && nSpace+1 < strLen) {
		strncpy_s(strTemp, 1024, str, nSpace);
		strcat_s(strTemp, 1024, "\n");
		strcat_s(strTemp, 1024, (str + nSpace + 1));

		strcpy_s(str, 1024, strTemp);
	}
}

//-----------------------------------------------------------------------------

void DisplaySongInfo()
{
	const static int nHeadlineChars = 19;
	const static int nText1Chars = 30;
	const static int nText2Chars = 30;

	WCHAR strUCS2[1024];
	char strUTF8[1024];
	char strHeadline[512];
	char strText1[1024];
	char strText2[512];

	long nTrack = QCDCallbacks->Service(opGetCurrentIndex, NULL, 0, 0);

	QCDCallbacks->Service(settings.Headline_ServiceOp, strUTF8, sizeof(strUTF8), nTrack);
	QCDCallbacks->Service(opUTF8toUCS2, strUTF8, (long)strUCS2, sizeof(strUTF8));
	WideCharToMultiByte(CP_ACP, 0, strUCS2, -1, strHeadline, 512, NULL, NULL);

	QCDCallbacks->Service(settings.Text1_ServiceOp, strUTF8, sizeof(strUTF8), nTrack);
	QCDCallbacks->Service(opUTF8toUCS2, strUTF8, (long)strUCS2, sizeof(strUTF8));
	WideCharToMultiByte(CP_ACP, 0, strUCS2, -1, strText1, 512, NULL, NULL);

	QCDCallbacks->Service(settings.Text2_ServiceOp, strUTF8, sizeof(strUTF8), nTrack);
	QCDCallbacks->Service(opUTF8toUCS2, strUTF8, (long)strUCS2, sizeof(strUTF8));
	WideCharToMultiByte(CP_ACP, 0, strUCS2, -1, strText2, 512, NULL, NULL);

	if (settings.bHeadline_wrap && strlen(strHeadline) > nHeadlineChars)
		InsertNextLine(strHeadline, nHeadlineChars);
	/*if (settings.bText1_wrap && strlen(strText1) > nText1Chars)
		InsertNextLine(strText1, nText1Chars);
	if (settings.bText2_wrap && strlen(strText2) > nText2Chars)
		InsertNextLine(strText2, nText2Chars);*/

	if (settings.Text2_ServiceOp != 0) {
		strcat_s(strText1, 1024, "\n");
		strcat_s(strText1, 1024, strText2);
	}

	if (!settings.bCascade && snIsMessageVisible(g_nMsgId))
		snHideMessage(g_nMsgId);
	g_nMsgId = snShowMessage(strHeadline, strText1, settings.nTimeout, g_strIcon);
}

//-----------------------------------------------------------------------------

bool IsPlaying()
{
	if (2 == QCDCallbacks->Service(opGetPlayerState, 0, 0, 0))
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{
	switch (msg) {
		case WM_PN_PLAYSTARTED :
		{
			DisplaySongInfo();
			break;
		}
		/*case WM_REGISTER_MSG :
			MessageBox(0, "test", "", 0);
			Configure(0);
			return 0;
		*/
	}
	
	return CallWindowProc(QCDProc, hwnd, msg, wparam, lparam);
}