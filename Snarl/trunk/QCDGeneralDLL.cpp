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

static const wchar_t PLUGIN_NAME[] = L"Snarl v1.4.5";


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


QCDModInitGen2    QCDCallbacks;
PluginServiceFunc Service;
HINSTANCE         hInstance  = NULL;
HWND              hwndPlayer = NULL;
WNDPROC           QCDProc    = NULL;
SnarlInterface*   snarl      = NULL;
_Settings         Settings;
QString           g_strDefaultIcon;

long              g_nConfigPageId = 0;
LONG32            g_nMsgId = 0;
LONG32            g_nSnarlGlobalMessage = 0;
LONG32            g_nSnarlVersion = 0;
UINT_PTR          g_nDelayTimerID = 0;

static const WCHAR INI_SECTION[] = L"Snarl";
static const int WM_REGISTER_MSG = 0; // 62091;

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
	QCDCallbacks.version			 = PLUGIN_API_VERSION_UNICODE;

	QCDCallbacks.toModule.Initialize  = Initialize;
	QCDCallbacks.toModule.ShutDown	 = ShutDown;
	QCDCallbacks.toModule.About		 = About;
	QCDCallbacks.toModule.Configure	 = Configure;

	return &QCDCallbacks;
}

//----------------------------------------------------------------------------

int Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = (char*)PLUGIN_NAME;
	Service = QCDCallbacks.Service;

	// Check Windows
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (osvi.dwMajorVersion < 5) {
		MessageBox(0, L"This version of Snarl plug-in for QMP will only run on Windows 2000 or later!",
			L"QMP:Snarl initialization error", MB_ICONSTOP);
		return FALSE;
	}

	// Check QMP version (b117+)
	if (Service(opGetPlayerVersion, NULL, 1, 0) < 117) {
		MessageBox(0, L"The latest beta builds of Quintessential Media Player\nis needed to run this Snarl plug-in.\n\nAt least build 117 required!",
			L"QMP:Snarl initialization error", MB_ICONSTOP);
		return FALSE;
	}

	hwndPlayer = (HWND)Service(opGetParentWnd, 0, 0, 0);

	LoadSettings();
	if (Settings.bStartSnarl && !snarl->snGetSnarlWindow())
		StartSnarl();
	CoverArtInitialize();

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

	g_nConfigPageId   = Service(opSetPluginPage, (void*)&ppp, 0, 0);

	// Create snarl object
	snarl = new SnarlInterface();
	if (SnarlInterface::M_OK != snarl->snRegisterConfig2(hwndPlayer, "Quintessential Media Player", WM_REGISTER_MSG, g_strDefaultIcon.GetUTF8()))
		OutputDebugInfo(L"Failed to register Snarl config");
	
	g_nSnarlGlobalMessage = snarl->snGetGlobalMsg();
	g_nSnarlVersion = snarl->snGetVersionEx();

	if (IsPlaying())
		DisplaySongInfo();

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags)
{
	QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDProc);
	Service(opRemovePluginPage, hInstance, g_nConfigPageId, 0);

	if (Settings.bCloseSnarl)
		CloseSnarl();
	else
		snarl->snRevokeConfig(hwndPlayer);
	delete snarl;

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
	strAbout += _T("\n\nPlug-in by:\nToke Noer (toke@noer.it)");
	MessageBox(hwndPlayer, strAbout , _T("About"), MB_OK | MB_ICONINFORMATION);
#else
	Test();
#endif 
}

//-----------------------------------------------------------------------------

void LoadSettings()
{
	WCHAR strTmp[MAX_PATH];
	WCHAR strIni[MAX_PATH];
	Service(opGetPluginSettingsFile, strIni, MAX_PATH*sizeof(WCHAR), 0);

	Settings.bDebug             = GetPrivateProfileIntW(INI_SECTION, L"bDebug", 0, strIni);
	Settings.nTimeout           = GetPrivateProfileIntW(INI_SECTION, L"nTimeout", 10, strIni);
	Settings.bCascade           = GetPrivateProfileIntW(INI_SECTION, L"bCascade", 0, strIni);
	Settings.bHeadline_wrap     = GetPrivateProfileIntW(INI_SECTION, L"bHeadline_Wrap", 0, strIni);
	Settings.bStartSnarl        = GetPrivateProfileIntW(INI_SECTION, L"bStartSnarl", 1, strIni);
	Settings.bCloseSnarl        = GetPrivateProfileIntW(INI_SECTION, L"bCloseSnarl", 0, strIni);
	//Settings.bText1_wrap         = GetPrivateProfileIntW(INI_SECTION, L"bText1_Wrap", 1, strIni);
	//Settings.bText2_wrap         = GetPrivateProfileIntW(INI_SECTION, L"bText2_Wrap", 0, strIni);

	Settings.Headline_ServiceOp = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bHeadline_Op", opGetArtistName, strIni);
	Settings.Text1_ServiceOp    = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bText1_Op", opGetTrackName, strIni);
	Settings.Text2_ServiceOp    = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bText2_Op", opGetDiscName, strIni);

	// Cover art
	Settings.bDisplayCoverArt = GetPrivateProfileIntW(INI_SECTION, L"bDisplayCoverArt", 1, strIni);
	GetPrivateProfileStringW(INI_SECTION, L"CoverArtRoot", L"%CURRENT_DIR", strTmp, MAX_PATH, strIni);
	Settings.strCoverArtRoot.assign(strTmp);
	GetPrivateProfileStringW(INI_SECTION, L"CoverArtTemplate", L"%A - %D - Front.%E", strTmp, MAX_PATH, strIni);
	Settings.strCoverArtTemplate.assign(strTmp);

	// Get icon path
	WCHAR szPluginfolder[MAX_PATH];
	Service(opGetPluginFolder, szPluginfolder, MAX_PATH*sizeof(WCHAR), 0);
	g_strDefaultIcon = szPluginfolder;
	g_strDefaultIcon.append(L"\\snarl.png");

	OutputDebugInfo(L"Defaul icon: %s", g_strDefaultIcon.GetUnicode());
}

//-----------------------------------------------------------------------------

void SaveSettings()
{
	static const size_t BUF_SIZE = 32;
	WCHAR buf[BUF_SIZE];
	WCHAR strIni[MAX_PATH];
	Service(opGetPluginSettingsFile, strIni, MAX_PATH*sizeof(WCHAR), 0);
	
	swprintf_s(buf, BUF_SIZE, L"%u", Settings.bDebug);             WritePrivateProfileStringW(INI_SECTION, L"bDebug",          buf, strIni);
	swprintf_s(buf, BUF_SIZE, L"%u", Settings.nTimeout);           WritePrivateProfileStringW(INI_SECTION, L"nTimeout",        buf, strIni);
	swprintf_s(buf, BUF_SIZE, L"%u", Settings.bCascade);           WritePrivateProfileStringW(INI_SECTION, L"bCascade",        buf, strIni);
	swprintf_s(buf, BUF_SIZE, L"%u", Settings.bHeadline_wrap);     WritePrivateProfileStringW(INI_SECTION, L"bHeadline_Wrap",  buf, strIni);
	swprintf_s(buf, BUF_SIZE, L"%u", Settings.bStartSnarl);        WritePrivateProfileStringW(INI_SECTION, L"bStartSnarl",     buf, strIni);
	swprintf_s(buf, BUF_SIZE, L"%u", Settings.bCloseSnarl);        WritePrivateProfileStringW(INI_SECTION, L"bCloseSnarl",     buf, strIni);

	swprintf_s(buf, BUF_SIZE, L"%u", Settings.Headline_ServiceOp); WritePrivateProfileStringW(INI_SECTION, L"bHeadline_Op",    buf, strIni);
	swprintf_s(buf, BUF_SIZE, L"%u", Settings.Text1_ServiceOp);    WritePrivateProfileStringW(INI_SECTION, L"bText1_Op",       buf, strIni);
	swprintf_s(buf, BUF_SIZE, L"%u", Settings.Text2_ServiceOp);    WritePrivateProfileStringW(INI_SECTION, L"bText2_Op",       buf, strIni);

	// Cover art
	swprintf_s(buf, BUF_SIZE, L"%u", Settings.bDisplayCoverArt);   WritePrivateProfileStringW(INI_SECTION, L"bDisplayCoverArt", buf, strIni);
	WritePrivateProfileStringW(INI_SECTION, L"CoverArtRoot",     Settings.strCoverArtRoot.GetUnicode(), strIni);
	WritePrivateProfileStringW(INI_SECTION, L"CoverArtTemplate", Settings.strCoverArtTemplate.GetUnicode(), strIni);
}

//-----------------------------------------------------------------------------

void InsertNextLine(QString* str, size_t nPos)
{
	if (nPos >= str->length())
		return;

	int nSpace = str->rfind(L" ", nPos);
	if (nSpace > 0)
		str->replace(nSpace, 1, L"\n");
}

//-----------------------------------------------------------------------------

void DisplaySongInfo()
{
	const static int nHeadlineChars = 26;
	const static int nText1Chars = 30;
	const static int nText2Chars = 30;

	if (!snarl)
		return;

	WCHAR strTmp[SnarlInterface::SNARL_STRING_LENGTH];
	QString strHeadline;
	QString strText1;
	QString strText2;
	QString strIcon;

	long nTrack = Service(opGetCurrentIndex, NULL, 0, 0);

	// Retrieve song information from QMP
	Service(Settings.Headline_ServiceOp, strTmp, SnarlInterface::SNARL_STRING_LENGTH, nTrack);
	strHeadline = strTmp;
	Service(Settings.Text1_ServiceOp,    strTmp, SnarlInterface::SNARL_STRING_LENGTH, nTrack);
	strText1 = strTmp;
	Service(Settings.Text2_ServiceOp,    strTmp, SnarlInterface::SNARL_STRING_LENGTH, nTrack);
	strText2 = strTmp;

	if (Settings.bHeadline_wrap && strHeadline.length() > nHeadlineChars)
		InsertNextLine(&strHeadline, nHeadlineChars);

	if (Settings.Text2_ServiceOp != 0) {
		strText1 += L"\n";
		strText1 += strText2;
	}

	if (Settings.bDisplayCoverArt)
		GetCoverArt(nTrack, &strIcon);

	// Hide if not cascade
	if (!Settings.bCascade && g_nMsgId > 0 && snarl->snIsMessageVisible(g_nMsgId))
		snarl->snUpdateMessage(g_nMsgId, strHeadline.GetUTF8(), strText1.GetUTF8(), (strIcon.length() > 0) ? strIcon.GetUTF8() : g_strDefaultIcon.GetUTF8());
	else
		g_nMsgId = snarl->snShowMessage(strHeadline.GetUTF8(), strText1.GetUTF8(), Settings.nTimeout, (strIcon.length() > 0) ? strIcon.GetUTF8() : g_strDefaultIcon.GetUTF8(), 0, 0);
}

//-----------------------------------------------------------------------------

bool IsPlaying()
{
	if (2 == Service(opGetPlayerState, 0, 0, 0))
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------

void CALLBACK DelayTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	KillTimer(NULL, g_nDelayTimerID);
	DisplaySongInfo();
}

//-----------------------------------------------------------------------------

void StartDisplayTimer(UINT nTime)
{
	if (g_nDelayTimerID) {
		KillTimer(NULL, g_nDelayTimerID);
		g_nDelayTimerID = 0;
	}
	g_nDelayTimerID = SetTimer(NULL, 0, nTime, DelayTimerProc);
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{
	if (msg == g_nSnarlGlobalMessage) {
		if (wparam == SnarlInterface::SNARL_LAUNCHED && snarl)
				snarl->snRegisterConfig2(hwndPlayer, "Quintessential Media Player", WM_REGISTER_MSG, g_strDefaultIcon.GetUTF8());
	}

	switch (msg)
	{
		case WM_PN_TRACKCHANGED :
			StartDisplayTimer(100);	// Workaround for CD track change
			break;

		case WM_PN_PLAYSTARTED :
			DisplaySongInfo();
			break;
	}

	return CallWindowProc(QCDProc, hwnd, msg, wparam, lparam);
}

//-----------------------------------------------------------------------------

void StartSnarl()
{
	HWND hWnd = NULL;
	QString strPath, strDirectory;

	// Check if already running
	if (IsWindow(SnarlInterface::snGetSnarlWindow()))
		return;

	// Get path from registry
	if (!GetSnarlPath(&strPath, &strDirectory))
		return;

	INT iError = (UINT)ShellExecute(NULL, L"open", strPath, NULL, strDirectory, SW_SHOWDEFAULT);
	if (iError <= 32) {
		QString strError;
		switch (iError) {
			case 0 :
				strError = L"The operating system is out of memory or resources.";
				break;
			case ERROR_FILE_NOT_FOUND :
				strError = L"The specified file was not found.";
				break;
			case ERROR_PATH_NOT_FOUND :
				strError = L"The specified path was not found.";
				break;
			case SE_ERR_ACCESSDENIED : 
				strError = L"The operating system denied access to the specified file.";
			default :
				strError = L"Unknown error. Error number: %d";
				break;
		}
		OutputDebugInfo(L"StartSnarl() : ShellExecute error : %s", strError.GetUnicode(), iError);
	}
	Sleep(0);
}

//-----------------------------------------------------------------------------

void CloseSnarl()
{
	int nCloseMsg = 0; 
	if (g_nSnarlVersion == 0)
		g_nSnarlVersion = snarl->snGetVersionEx();

	if (g_nSnarlVersion <= 37)
		nCloseMsg = WM_USER + 81;
	else
		nCloseMsg = WM_CLOSE;

	DWORD nReturn = 0;
	HWND hWnd = SnarlInterface::snGetSnarlWindow();
	if (IsWindow(hWnd))
		SendMessageTimeout(hWnd, nCloseMsg, 0, 0, SMTO_ABORTIFHUNG, 1000, &nReturn);
}

//-----------------------------------------------------------------------------

BOOL GetSnarlPath(QString* strPath, QString* strDir)
{
	const static WCHAR* hSubKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Snarl.exe";
	DWORD nPathSize = MAX_PATH*sizeof(WCHAR);
	WCHAR strTemp[MAX_PATH] = {0};
	DWORD dwBufLen = MAX_PATH;
	
	
	//LONG nRet = RegGetValue(HKEY_LOCAL_MACHINE, hSubKey, NULL, RRF_RT_REG_SZ, NULL, strTemp, &nPathSize);
	HKEY hKey = 0;
	LONG nRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, hSubKey, 0, KEY_QUERY_VALUE, &hKey);
	if (nRet == ERROR_SUCCESS)
	{
		LONG lRet = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)strTemp, &dwBufLen);
		RegCloseKey(hKey);

		if((lRet != ERROR_SUCCESS) || (dwBufLen >= MAX_PATH))
			return FALSE;

		strPath->assign(strTemp);
		int nPos = strPath->rfind(L"\\");
		strDir->assign(strPath->substr(0, nPos+1));
		return TRUE;
	}
	return FALSE;
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

	strLog[nCopiedChars+10] = L'\n';
	strLog[nCopiedChars+11] = NULL;
	
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