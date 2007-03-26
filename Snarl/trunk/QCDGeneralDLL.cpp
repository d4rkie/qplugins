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
//   All string routines are uckly hacks and should be compile against
//   Unicows.dll instead!
//-----------------------------------------------------------------------------

#define PLUGIN_NAME "Snarl v1.3"


#define UNICODE 1
#define _UNICODE 1
#define _WIN32_WINNT 0x0500
#define WINVER       0x0500

#include <TCHAR.H>
#include <QString.h>

#include <IQCDTagInfo.h>

#include "CoverArt.h"
#include "Config.h"
#include "QCDGeneralDLL.h"


QCDModInitGen2   QCDCallbacks;
HINSTANCE        hInstance  = NULL;
HWND             hwndPlayer = NULL;
WNDPROC          QCDProc    = NULL;
SnarlInterface*  snarl      = NULL;
Settings         settings;
QString          g_strDefaultIcon;

long             g_nConfigPageId = 0;
LONG32	         g_nMsgId = 0;

static const WCHAR INI_SECTION[] = L"Snarl";
static const int WM_REGISTER_MSG = 62091;

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
	QCDCallbacks.size				 = sizeof(QCDModInitGen2);
	QCDCallbacks.version			 = PLUGIN_API_VERSION_NTUTF8;

	QCDCallbacks.toModule.Initialize = Initialize;
	QCDCallbacks.toModule.ShutDown	 = ShutDown;
	QCDCallbacks.toModule.About		 = About;
	QCDCallbacks.toModule.Configure	 = Configure;

	return &QCDCallbacks;
}

//----------------------------------------------------------------------------

int Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = PLUGIN_NAME;

	hwndPlayer = (HWND)QCDCallbacks.Service(opGetParentWnd, 0, 0, 0);
	LoadSettings();

	snarl = new SnarlInterface();
	snarl->snRegisterConfig2(hwndPlayer, "Quintessential Media Player", WM_REGISTER_MSG, g_strDefaultIcon.GetUTF8());

	// Subclass the player and listen for WM_PN_? and WM_REGISTER_MSG
	if ((QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDSubProc)) == 0) {
		MessageBox(hwndPlayer, _T("Failed to subclass player!"), _T("Fatal init error!"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	// Create config pages
	PluginPrefPage ppp;
	ppp.struct_size   = sizeof(PluginPrefPage);
	ppp.hModule       = hInstance;
	ppp.lpTemplate    = MAKEINTRESOURCE(IDD_CONFIG2);
	ppp.lpDialogFunc  = ConfigDlgProc;
	ppp.lpDisplayText = L"Snarl notification";
	ppp.nCategory     = PREFPAGE_CATEGORY_GENERAL;
	ppp.hModuleParent = NULL;
	ppp.groupID       = 0;
	ppp.createParam   = 0;

	g_nConfigPageId = QCDCallbacks.Service(opSetPluginPage, (void*)&ppp, 0, 0);

	if (IsPlaying())
		DisplaySongInfo();

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDProc);
	QCDCallbacks.Service(opRemovePluginPage, hInstance, g_nConfigPageId, 0);
	snarl->snRevokeConfig(hwndPlayer);
	delete snarl;
	SaveSettings();
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	QCDCallbacks.Service(opShowPluginPage, hInstance, g_nConfigPageId, 0);

}

//-----------------------------------------------------------------------------

void About(int flags)
{
#if 1
	QString str;
	str.SetMultiByte(PLUGIN_NAME);
	str += _T("\n\nPlug-in by:\nToke Noer (toke@noer.it)");
	MessageBox(hwndPlayer, str , _T("About"), MB_OK | MB_ICONINFORMATION);

#else
	Test();
#endif 
}

//-----------------------------------------------------------------------------

void LoadSettings()
{
	WCHAR strIni[MAX_PATH];
	WCHAR strTmp[MAX_PATH];
	QCDCallbacks.Service(opGetPluginSettingsFile, strTmp, MAX_PATH*sizeof(WCHAR), 0); // Returns UTF8
	QCDCallbacks.Service(opUTF8toUCS2, strTmp, (long)strIni, MAX_PATH);

	settings.nTimeout           = GetPrivateProfileIntW(INI_SECTION, L"nTimeout", 10, strIni);
	settings.bCascade           = GetPrivateProfileIntW(INI_SECTION, L"bCascade", 0, strIni);
	settings.bHeadline_wrap     = GetPrivateProfileIntW(INI_SECTION, L"bHeadline_Wrap", 0, strIni);
	//settings.bText1_wrap         = GetPrivateProfileIntW(INI_SECTION, L"bText1_Wrap", 1, strIni);
	//settings.bText2_wrap         = GetPrivateProfileIntW(INI_SECTION, L"bText2_Wrap", 0, strIni);
	settings.Headline_ServiceOp = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bHeadline_Op", opGetArtistName, strIni);
	settings.Text1_ServiceOp    = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bText1_Op", opGetTrackName, strIni);
	settings.Text2_ServiceOp    = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bText2_Op", opGetDiscName, strIni);

	// Cover art
	settings.bDisplayCoverArt = GetPrivateProfileIntW(INI_SECTION, L"bDisplayCoverArt", 1, strIni);
	GetPrivateProfileStringW(INI_SECTION, L"CoverArtRoot", L"%CURRENT_DIR", strTmp, MAX_PATH, strIni);
	settings.strCoverArtRoot.assign(strTmp);
	GetPrivateProfileStringW(INI_SECTION, L"CoverArtTemplate", L"%A - %D - Front.%E", strTmp, MAX_PATH, strIni);
	settings.strCoverArtTemplate.assign(strTmp);

	// Get icon path
	char szPluginfolder[MAX_PATH];
	QCDCallbacks.Service(opGetPluginFolder, szPluginfolder, MAX_PATH, 0);
	g_strDefaultIcon.SetUTF8(szPluginfolder);
	g_strDefaultIcon.append(L"\\snarl.png");

	swprintf(strTmp, L"Snarl >> Defaul icon: %s", g_strDefaultIcon.GetUnicode());
	OutputDebugStringW(strTmp);
}

//-----------------------------------------------------------------------------

void SaveSettings()
{
	WCHAR buf[32];
	WCHAR strIni[MAX_PATH];
	WCHAR strTmp[MAX_PATH];
	QCDCallbacks.Service(opGetPluginSettingsFile, strTmp, MAX_PATH*sizeof(WCHAR), 0); // Returns UTF8
	QCDCallbacks.Service(opUTF8toUCS2, strTmp, (long)strIni, MAX_PATH);
	
	wsprintfW(buf, L"%u", settings.nTimeout);          	WritePrivateProfileStringW(INI_SECTION, L"nTimeout",        buf, strIni);
	wsprintfW(buf, L"%u", settings.bCascade);          	WritePrivateProfileStringW(INI_SECTION, L"bCascade",        buf, strIni);
	wsprintfW(buf, L"%u", settings.bHeadline_wrap);     WritePrivateProfileStringW(INI_SECTION, L"bHeadline_Wrap",  buf, strIni);
	//wsprintfW(buf, L"%u", settings.bText1_wrap);        WritePrivateProfileStringW(INI_SECTION, L"bText1_Wrap",     buf, strIni);
	//wsprintfW(buf, L"%u", settings.bText2_wrap);        WritePrivateProfileStringW(INI_SECTION, L"bText2_Wrap",     buf, strIni);
	wsprintfW(buf, L"%u", settings.Headline_ServiceOp); WritePrivateProfileStringW(INI_SECTION, L"bHeadline_Op",    buf, strIni);
	wsprintfW(buf, L"%u", settings.Text1_ServiceOp);    WritePrivateProfileStringW(INI_SECTION, L"bText1_Op",       buf, strIni);
	wsprintfW(buf, L"%u", settings.Text2_ServiceOp);    WritePrivateProfileStringW(INI_SECTION, L"bText2_Op",       buf, strIni);

	// Cover art
	wsprintfW(buf, L"%u", settings.bDisplayCoverArt);   WritePrivateProfileStringW(INI_SECTION, L"bDisplayCoverArt", buf, strIni);
	WritePrivateProfileStringW(INI_SECTION, L"CoverArtRoot",     settings.strCoverArtRoot.GetUnicode(), strIni);
	WritePrivateProfileStringW(INI_SECTION, L"CoverArtTemplate", settings.strCoverArtTemplate.GetUnicode(), strIni);
}

//-----------------------------------------------------------------------------

int FindSpaceRew(LPSTR str, int nPos)
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

void InsertNextLine(LPSTR str, int nPos)
{
	char strTemp[1024];
	int strLen = strlen(str);
	int nSpace = FindSpaceRew(str, nPos); // Find space before edge
	
	/*char strD[1024];
	sprintf(strD, "nSpace: %d", nSpace);
	MessageBox(hwndPlayer, strD, "Debug", 0);*/

	if (nSpace > 5 && (nSpace+1 < strLen) )
	{
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

	char strHeadline[SnarlInterface::SNARL_STRING_LENGTH];
	char strText1[SnarlInterface::SNARL_STRING_LENGTH];
	char strText2[SnarlInterface::SNARL_STRING_LENGTH];
	char strIcon[MAX_PATH] = "";

	long nTrack = QCDCallbacks.Service(opGetCurrentIndex, NULL, 0, 0);

	QCDCallbacks.Service(settings.Headline_ServiceOp, strHeadline, SnarlInterface::SNARL_STRING_LENGTH, nTrack);
	QCDCallbacks.Service(settings.Text1_ServiceOp, strText1, SnarlInterface::SNARL_STRING_LENGTH, nTrack);
	QCDCallbacks.Service(settings.Text2_ServiceOp, strText2, SnarlInterface::SNARL_STRING_LENGTH, nTrack);

	if (settings.bHeadline_wrap && strnlen(strHeadline, sizeof(strHeadline)) > nHeadlineChars)
		InsertNextLine(strHeadline, nHeadlineChars);

	if (settings.Text2_ServiceOp != 0) {
		strcat_s(strText1, SnarlInterface::SNARL_STRING_LENGTH, "\n");
		strcat_s(strText1, SnarlInterface::SNARL_STRING_LENGTH, strText2);
	}

	if (settings.bDisplayCoverArt)
		GetCoverArt(nTrack, strIcon);

	// Hide if not cascade
	if (!settings.bCascade && g_nMsgId > 0 && snarl->snIsMessageVisible(g_nMsgId))
		snarl->snHideMessage(g_nMsgId);
	g_nMsgId = snarl->snShowMessage(strHeadline, strText1, settings.nTimeout, (strIcon[0] != '\0') ? strIcon : g_strDefaultIcon.GetUTF8(), 0, 0);
	
	CHAR strDbg[128 + MAX_PATH];
	sprintf(strDbg, "Snarl >> MsgId: %u\nPath: %s", g_nMsgId, strIcon);
	OutputDebugStringA(strDbg);
}

//-----------------------------------------------------------------------------

bool IsPlaying()
{
	if (2 == QCDCallbacks.Service(opGetPlayerState, 0, 0, 0))
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
		case WM_REGISTER_MSG :
			// MessageBox(0, _T("WM_REGISTER_MSG"), _T(""), 0);
			Configure(0);
			break;
	}

	return CallWindowProc(QCDProc, hwnd, msg, wparam, lparam);
}


//-----------------------------------------------------------------------------

void Test()
{
	LPCTSTR str1 = snarl->snGetAppPath();
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
	snarl->snSetTimeout(id, 10);
}