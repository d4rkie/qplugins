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
//
// Plug-in by Toke Noer (toke@noer.it)
// Notes:
//   debug.h can found at this article:
//   http://www.codeproject.com/debug/debug_macros.asp
//
// Todo:
//	
//
//-----------------------------------------------------------------------------

#include <Windows.h>
#include <stdio.h>
#include <TCHAR.h>
#include <debug.h>
#include "QCDGeneralDLL.h"
#include "Config.h"
#include "About.h"

#pragma warning(disable:4002)		// Disable "too many actual parameters for macro INFO"

// Constants
static       CHAR*   PLUGIN_NAME   = "MSN \"What I'm Listening To\" v2.5.1";
static const WCHAR   INI_SECTION[] = L"MSN-WILT";
static const WCHAR*  REGKEY_MSN    = L"Software\\Microsoft\\MSNMessenger";
static const WCHAR*  REGKEY_WMP    = L"Software\\Microsoft\\Active Setup\\Installed Components\\{6BF52A52-394A-11d3-B153-00C04F79FAA6}";
static const WORD    TITLE_LEN     = 100;
static const WORD    ALBUM_LEN     = 100;
static const WORD    ARTIST_LEN    = 100;
static const WORD    CONTENTID_LEN = 100;

static const LONG DIGITAL_AUDIOFILE_MEDIA_QMP = 0x1000;
static const LONG DIGITAL_VIDEOFILE_MEDIA_QMP = 0x10000;

// Global variables
HINSTANCE		hInstance;
HWND			hwndPlayer;
QCDModInitGen	*QCDCallbacks;
WNDPROC			QCDProc;

BOOL			bIsEncoding;
INT				nMSNBuild;
UINT_PTR		nDelayTimerID;
WCHAR*			strTrackPlaying;
Settings		settings;

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
	// use this version for new style API calls (all returned text in UTF8 encoding on WinNT/2K/XP (native encoding on Win9x))
	ModInit->version				= PLUGIN_API_VERSION_WANTUTF8;
	ModInfo->moduleString			= PLUGIN_NAME;

	ModInit->toModule.ShutDown		= ShutDown;
	ModInit->toModule.About			= About;
	ModInit->toModule.Configure		= Configure;

	QCDCallbacks = ModInit;

	hwndPlayer = (HWND)QCDCallbacks->Service(opGetParentWnd, 0, 0, 0);

	// TODO: all your plugin initialization here
	nDelayTimerID = 0;
	bIsEncoding = FALSE;

	strTrackPlaying = new WCHAR[MAX_PATH];
	if (!strTrackPlaying)
		return FALSE;

	// Subclass the player and listen for WM_PN_?
	//if ((QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDSubProc)) == 0) {
	if ((QCDProc = (WNDPROC)SetWindowLongPtr(hwndPlayer, GWLP_WNDPROC, (LONG_PTR)QCDSubProc)) == 0){
		MessageBox(hwndPlayer, _T("Failed to subclass player!"), _T("Fatal init error!"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	nMSNBuild = RegDB_GetMSNBuild();
	LoadSettings();

	// QCD playing
	if (IsPlaying()) {
		MSNMessages msn;
		ZeroMemory(&msn, sizeof(msn));
		CurrentSong(&msn);
		msn.msncommand = 1;
		
		SendToMSN(&msn);
	}

	return TRUE; // return TRUE for successful initialization
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	// Remove subclassing
	QCDProc = (WNDPROC)SetWindowLongPtr(hwndPlayer, GWLP_WNDPROC, (LONG_PTR)QCDProc);

	if (nDelayTimerID)
		KillTimer(NULL, nDelayTimerID);
	ClearSong();
	QBlog_CleanUpRegDB();
	SaveSettings();

	delete [] strTrackPlaying;
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
	HWND hwnd = (HWND)QCDCallbacks->Service(opGetPropertiesWnd, NULL, 0, 0);
	if (!hwnd)
		hwnd = hwndPlayer;
	CAbout dlg(hInstance, hwnd);
}

//-----------------------------------------------------------------------------
// Program functions
//-----------------------------------------------------------------------------

void LoadSettings()
{
	BOOL bFirstRun = TRUE;
	WCHAR strIni[MAX_PATH];
	WCHAR strTmp[MAX_PATH];
	QCDCallbacks->Service(opGetPluginSettingsFile, strTmp, MAX_PATH*sizeof(WCHAR), 0); // Returns UTF8
	QCDCallbacks->Service(opUTF8toUCS2, strTmp, (long)strIni, MAX_PATH);

	settings.bTitle  = TRUE; // GetPrivateProfileInt(INI_SECTION, L"bTitle", 1, strIni);
	settings.bArtist = GetPrivateProfileInt(INI_SECTION, L"bArtist", 1, strIni);
	settings.bAlbum  = GetPrivateProfileInt(INI_SECTION, L"bAlbum",  0, strIni);
	settings.bVideo  = GetPrivateProfileInt(INI_SECTION, L"bVideo",  0, strIni);
	settings.nDelay  = GetPrivateProfileInt(INI_SECTION, L"nDelay",  0, strIni);

	bFirstRun        = GetPrivateProfileInt(INI_SECTION, L"bFirstRun", 1, strIni);
	if (bFirstRun) {
		if (RegDB_GetWMPVersion() < 9) {
			if (IDYES == MessageBox(hwndPlayer, 
					L"The \"MSN - What Im Listening To\" QMP plug-in has detected that WMP (Windows Media Player)\n" \
					L"isn't installed on your computer or you are running an old version which isn't supported.\n\n" \
					L"In order to turn on the \"What Im Listening To\" feature in MSN you have to either:\n" \
					L"  1) Install Windows Media Player\n"\
					L"  2) Let this plug-in fix it without need of WMP\n\n" \
					L"Do you want to let the plug-in fix the problem now?\n" \
					L"(This can be undone from the configuration dialog of this plug-in.)", 
					L"QMP \"What Im Listening To\"", MB_YESNO | MB_ICONQUESTION))
			{
				RegDB_Fix(TRUE);
				
			}
		}
		WritePrivateProfileString(INI_SECTION, L"bFirstRun", L"0", strIni);
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
	
	// wsprintf(buf, L"%d", settings.bTitle);	WritePrivateProfileString(INI_SECTION, L"bTitle",  buf, strIni);
	wsprintf(buf, L"%d", settings.bArtist);	WritePrivateProfileString(INI_SECTION, L"bArtist", buf, strIni);
	wsprintf(buf, L"%d", settings.bAlbum);	WritePrivateProfileString(INI_SECTION, L"bAlbum",  buf, strIni);
	wsprintf(buf, L"%d", settings.bVideo);	WritePrivateProfileString(INI_SECTION, L"bVideo",  buf, strIni);
	wsprintf(buf, L"%u", settings.nDelay);	WritePrivateProfileString(INI_SECTION, L"nDelay",  buf, strIni);
}

//-----------------------------------------------------------------------------
// RegDB functions for faking WMP 10
//-----------------------------------------------------------------------------
void RegDB_Fix(BOOL bFix)
{
	static BOOL bIsFixed = FALSE;

	if (bFix) {
		// Check if entries already present
		if (RegDB_GetWMPVersion() < 9)
			RegDB_Insert();
	}
	else
			RegDB_Clean();
}

//-----------------------------------------------------------------------------

void RegDB_Insert()
{
	HKEY hKey = NULL;
	DWORD dwDis = NULL;
	LPTSTR lpClass = _T("");
	
	SECURITY_ATTRIBUTES lpSecurityAtt;
	lpSecurityAtt.nLength = sizeof(LPSECURITY_ATTRIBUTES);
	lpSecurityAtt.lpSecurityDescriptor = NULL;
	lpSecurityAtt.bInheritHandle = TRUE;

	if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGKEY_WMP, 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, &lpSecurityAtt, &hKey, &dwDis))
	{
		DWORD nValue;

		RegSetValueEx(hKey, _T(""),            0, REG_SZ,    (CONST BYTE*)_T("Microsoft Windows Media Player"), 31*sizeof(_TCHAR));
		RegSetValueEx(hKey, _T("ComponentID"), 0, REG_SZ,    (CONST BYTE*)_T("Microsoft Windows Media Player"), 31*sizeof(_TCHAR));
		RegSetValueEx(hKey, _T("Locale"),      0, REG_SZ,    (CONST BYTE*)_T("EN"), 3*sizeof(_TCHAR));
		RegSetValueEx(hKey, _T("StubPath"),    0, REG_SZ,    (CONST BYTE*)_T(""), 1*sizeof(_TCHAR));
		RegSetValueEx(hKey, _T("Version"),     0, REG_SZ,    (CONST BYTE*)_T("10,0,0,3646"), 12*sizeof(_TCHAR));

		nValue = 2; RegSetValueEx(hKey, _T("DontAsk"),     0, REG_DWORD, (CONST BYTE*)&nValue, 4);
		nValue = 1; RegSetValueEx(hKey, _T("IsInstalled"), 0, REG_DWORD, (CONST BYTE*)&nValue, 4);

		RegCloseKey(hKey);
	}
}

//-----------------------------------------------------------------------------

void RegDB_Clean()
{
	RegDeleteKey(HKEY_LOCAL_MACHINE, REGKEY_WMP);
}

//-----------------------------------------------------------------------------

INT RegDB_GetWMPVersion()
{
	UINT nVersion = 0;
	HKEY hKey = NULL;

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_WMP, 0,  KEY_READ, &hKey)) {
		BYTE strBuff[64];
		DWORD nBuffSize = 64;
		DWORD nType = 0;
		_TCHAR* pPos = NULL;

		RegQueryValueEx(hKey, L"Version", NULL, &nType, strBuff, &nBuffSize);		
		RegCloseKey(hKey);

		if (nBuffSize == 64) // Might not be 0 terminated
			strBuff[63] = 0;

		if (nBuffSize > 0) {
			pPos = wcsstr((WCHAR*)strBuff, L",");
			
			if (pPos) {
				*pPos = 0;
				nVersion = _ttoi((WCHAR*)strBuff);
			}
		}
	}
	return nVersion;
}

//-----------------------------------------------------------------------------
// Returns MSNMessenger build version
//-----------------------------------------------------------------------------
INT RegDB_GetMSNBuild()
{
	UINT nBuild = 0;
	HKEY hKey = NULL;

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_MSN, 0,  KEY_READ, &hKey)) {
		WCHAR strBuff[64];
		DWORD nBuffSize = 64;
		DWORD nType = 0;
		WCHAR* pPos = NULL;

		RegQueryValueEx(hKey, L"AppCompatCanary", NULL, &nType, (LPBYTE)strBuff, &nBuffSize);
		RegCloseKey(hKey);

		if (nBuffSize == 64) // Might not be 0 terminated
			strBuff[63] = L'\0';

		if (nBuffSize > 0) {
			pPos = wcsrchr(strBuff, L'.');
			
			if (pPos) {
				pPos++;
				nBuild = _wtoi(pPos);
			}
		}
	}
	return nBuild;
}

//-----------------------------------------------------------------------------
// MSN and song related functions
//-----------------------------------------------------------------------------
void SendToMSN(MSNMessages *msn)
{
	INFO("SendToMSN()");

	// Send empty if not showing video info
	if (settings.bVideo == FALSE) {
		long nReturn = QCDCallbacks->Service(opGetMediaType, 0, -1, 0);
		if(nReturn == DIGITAL_VIDEOFILE_MEDIA || nReturn == DIGITAL_VIDEOFILE_MEDIA_QMP) {
			ZeroMemory(msn, sizeof(MSNMessages));
		}
	}

	// Get the right string patterns
	WCHAR strMSNFormat[64] = {0};
	static const WCHAR* MSNMusicStr566 = L"\\0Music\\0%d\\0%s\\0%s\\0%s\\0%s\\0";		// MSN 8 build 566+
	static const WCHAR* MSNMusicStr    = L"\\0Music\\0%d\\0%s\\0%s\\0%s\\0%s\\0%s\\0";	// MSN 7.5
	const WCHAR* MSNMusicString = NULL;

	if (nMSNBuild == 566)
		MSNMusicString = MSNMusicStr566;
	else
		MSNMusicString = MSNMusicStr;
	
	// Format string
	if (settings.bArtist && wcslen(msn->artist) > 0)
		lstrcatW(strMSNFormat, L"{1} - ");
	if (settings.bAlbum && wcslen(msn->album) > 0)
		lstrcatW(strMSNFormat, L"{2} - ");
	lstrcatW(strMSNFormat, L"{0}");
	
	HWND msnui = NULL;
	COPYDATASTRUCT msndata;
	WCHAR strBuffer[TITLE_LEN + ALBUM_LEN + ARTIST_LEN + CONTENTID_LEN + 4 + 32];
	ZeroMemory(&strBuffer, sizeof(strBuffer));

	if (nMSNBuild == 566)
		swprintf_s(strBuffer, sizeof(strBuffer)/sizeof(WCHAR), MSNMusicString, msn->msncommand, msn->title, msn->artist, msn->album, msn->wmcontentid);
	else
		swprintf_s(strBuffer, sizeof(strBuffer)/sizeof(WCHAR), MSNMusicString, msn->msncommand, strMSNFormat, msn->title, msn->artist, msn->album, msn->wmcontentid);

	msndata.dwData = 0x547;
	msndata.lpData = &strBuffer;
	msndata.cbData = (lstrlenW(strBuffer) + 1) * 2;	// Size in bytes

	while (msnui = FindWindowEx(NULL, msnui, L"MsnMsgrUIManager", NULL))
	{
		SendMessage(msnui, WM_COPYDATA, NULL, (LPARAM)&msndata);
	}
}

//-----------------------------------------------------------------------------

void CurrentSong(MSNMessages *msn)
{
	INFO("CurrentSong()");
	static const long STRSIZE = 100;
	WCHAR strTemp[STRSIZE];
	WCHAR strTitle[STRSIZE];
	WCHAR strArtist[STRSIZE];
	WCHAR strAlbum[STRSIZE];

	ZeroMemory(strTitle,  STRSIZE*sizeof(WCHAR));
	ZeroMemory(strArtist, STRSIZE*sizeof(WCHAR));
	ZeroMemory(strAlbum,  STRSIZE*sizeof(WCHAR));

	QCDCallbacks->Service(opGetTrackName, strTemp, STRSIZE*sizeof(WCHAR), -1);
	QCDCallbacks->Service(opUTF8toUCS2, strTemp, (long)strTitle, STRSIZE);

	QCDCallbacks->Service(opGetArtistName, strTemp, STRSIZE*sizeof(WCHAR), -1);
	QCDCallbacks->Service(opUTF8toUCS2, strTemp, (long)strArtist, STRSIZE);

	QCDCallbacks->Service(opGetDiscName, strTemp, STRSIZE*sizeof(WCHAR), -1);
	QCDCallbacks->Service(opUTF8toUCS2, strTemp, (long)strAlbum, STRSIZE);

	lstrcpyW(msn->artist, strArtist);
	lstrcpyW(msn->title, strTitle);
	lstrcpyW(msn->album, strAlbum);
	lstrcpyW(msn->wmcontentid, L"");
}

//-----------------------------------------------------------------------------

void UpdateSong()
{
	INFO("UpdateSong()");
	WCHAR strTemp[MAX_PATH];
	ZeroMemory(strTrackPlaying, MAX_PATH*sizeof(WCHAR));
	QCDCallbacks->Service(opGetTrackFile, strTemp, MAX_PATH*sizeof(WCHAR), -1);
	QCDCallbacks->Service(opUTF8toUCS2, strTemp, (long)strTrackPlaying, MAX_PATH);

	MSNMessages msn;
	ZeroMemory(&msn, sizeof(msn));
	CurrentSong(&msn);
	msn.msncommand = 1;
	SendToMSN(&msn);

	QBlog_InsertInRegDB();
}

//-----------------------------------------------------------------------------

void ClearSong()
{
	MSNMessages msn;
	ZeroMemory(&msn, sizeof(msn));
	SendToMSN(&msn);

	QBlog_CleanUpRegDB();
}

//-----------------------------------------------------------------------------

void StartTimer(UINT nForced)
{
	if (nDelayTimerID) {
		KillTimer(NULL, nDelayTimerID);
		nDelayTimerID = 0;
	}

	if (settings.nDelay > 0 || nForced > 0)
		nDelayTimerID = SetTimer(NULL, 0, (settings.nDelay >= nForced) ? settings.nDelay : nForced, DelayTimerProc);
	else
		UpdateSong();
}

//-----------------------------------------------------------------------------

BOOL IsPlaying()
{
	long iState = QCDCallbacks->Service(opGetPlayerState, 0, 0, 0);
	if (iState == 2)
		return TRUE;
	return FALSE;
}
//-----------------------------------------------------------------------------
// QBlog functions
//
// When you play a song, the plugin records the following information into the registry.
// "Title" - The title of the song. If not defined, then it will be set to the current file name.
// "DurationString" - The running time of this track in the format %02d:%02d (ie minutes:seconds, both padded to two digits).
// "Author" - The author the track, if defined. Otherwise this does not exist.
// "Album" - The album of the track, if defined. Otherwise this does not exist.
// When you stop playing, that information is deleted. 
//-----------------------------------------------------------------------------
void QBlog_InsertInRegDB()
{
	HKEY hKey = NULL;
	DWORD dwDis = NULL;
	LPTSTR lpClass = _T("");
	
	SECURITY_ATTRIBUTES lpSecurityAtt;
	lpSecurityAtt.nLength = sizeof(LPSECURITY_ATTRIBUTES);
	lpSecurityAtt.lpSecurityDescriptor = NULL;
	lpSecurityAtt.bInheritHandle = TRUE;

	if (IsPlaying()) {
		if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\MediaPlayer\\CurrentMetadata"), 
			0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE,	&lpSecurityAtt, &hKey, &dwDis)) {
			
			char  strUTF8[MAX_PATH];
			WCHAR strUCS2[MAX_PATH];
			DWORD cbSize = 0;

			// == Author
			if (1 == QCDCallbacks->Service(opGetArtistName, &strUTF8, MAX_PATH, -1)) {
				QCDCallbacks->Service(opUTF8toUCS2, strUTF8, (long)strUCS2, MAX_PATH);
				cbSize = wcslen(strUCS2) * sizeof(WCHAR);
				RegSetValueExW(hKey, _T("Author"), 0, REG_SZ, (LPBYTE)strUCS2, cbSize);
			}
			// == Album
			if (1 == QCDCallbacks->Service(opGetDiscName, &strUTF8, MAX_PATH, -1)) {
				QCDCallbacks->Service(opUTF8toUCS2, strUTF8, (long)strUCS2, MAX_PATH);
				cbSize = wcslen(strUCS2) * sizeof(WCHAR);
				RegSetValueExW(hKey, _T("Album"), 0, REG_SZ, (LPBYTE)strUCS2, cbSize);
			}
			// == Title
			if (1 == QCDCallbacks->Service(opGetTrackName, &strUTF8, MAX_PATH, -1)) {
				QCDCallbacks->Service(opUTF8toUCS2, strUTF8, (long)strUCS2, MAX_PATH);
				cbSize = wcslen(strUCS2) * sizeof(WCHAR);
				RegSetValueExW(hKey, _T("Title"), 0, REG_SZ, (LPBYTE)strUCS2, cbSize);
			}

			// == Duration
			long iTime = QCDCallbacks->Service(opGetTrackLength, NULL, -1, 0);
			long iMinutes = iTime/60;
			long iSeconds = iTime%60;

			if (iMinutes < 10 && iSeconds < 10)
				_stprintf_s((LPTSTR)strUCS2, MAX_PATH, _T("0%d:0%d"), iMinutes, iSeconds);
			else if (iMinutes > 9 && iSeconds < 10)
				_stprintf_s((LPTSTR)strUCS2, MAX_PATH, _T("%d:0%d"), iMinutes, iSeconds);
			else if (iMinutes < 10 && iSeconds > 9)
				_stprintf_s((LPTSTR)strUCS2, MAX_PATH, _T("0%d:%d"), iMinutes, iSeconds);
			else
				_stprintf_s((LPTSTR)strUCS2, MAX_PATH, _T("%d:%d"), iMinutes, iSeconds);
		
			cbSize = wcslen(strUCS2) * sizeof(WCHAR);
			RegSetValueExW(hKey, _T("DurationString"), 0, REG_SZ, (LPBYTE)strUCS2, cbSize);

			// == Clode RegDB
			RegCloseKey(hKey);
		}
	}
}

//-----------------------------------------------------------------------------

void QBlog_CleanUpRegDB()
{
	HKEY hKey = NULL;

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\MediaPlayer\\CurrentMetadata"), 0,  KEY_WRITE, &hKey)) {
		RegDeleteValue(hKey, _T("Author"));
		RegDeleteValue(hKey, _T("Album"));
		RegDeleteValue(hKey, _T("Title"));
		RegDeleteValue(hKey, _T("DurationString"));
	}			
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

void CALLBACK DelayTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	KillTimer(NULL, nDelayTimerID);
	UpdateSong();
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{ 
	switch (msg) {
		// Dont show on encoding
		/*case WM_PN_ENCODEPROGRESS :
		case WM_PN_ENCODESTARTED :
		case WM_PN_ENCODEPAUSED :
			bIsEncoding = TRUE;
			MessageBox(0, "Test", "", 0);
			break;

		case WM_PN_ENCODESTOPPED :
		case WM_PN_ENCODEDONE :
			bIsEncoding = FALSE;
			break;
		*/
		// Set info
		case WM_PN_INFOCHANGED : {
			INFO("-WM_PN_INFOCHANGED: wparam: %X, lparam: %X", wparam, lparam);

			// Check that it's not a cd/dvd that was inserted
			if (lparam <= 'Z')
				break;
			if (QCDCallbacks->Service(opGetPlayerState, 0, 0, 0) == 1) // Stopped
				break;
			// Check if the info has changed for the playing track
			if (lstrcmpW((LPCWSTR)lparam, strTrackPlaying) != 0)
				break;
		}
			// Fall through
		case WM_PN_TRACKCHANGED :
			INFO("-WM_PN_TRACKCHANGED: wparam: %X, lparam: %X", wparam, lparam);
			if (!bIsEncoding)
				StartTimer(1);
			break;

		case WM_PN_PLAYSTARTED :
		{
			INFO("-WM_PN_PLAYSTARTED: wparam: %X, lparam: %X", wparam, lparam);
			if (!bIsEncoding)
				StartTimer();
			break;
		}
		case WM_PN_PLAYSTOPPED :
			INFO("-WM_PN_PLAYSTOPPED: wparam: %X, lparam: %X", wparam, lparam);
			// Fallthrough
		case WM_PN_PLAYDONE :
			INFO("-WM_PN_PLAYDONE: wparam: %X, lparam: %X", wparam, lparam);
			ZeroMemory(strTrackPlaying, MAX_PATH);
			ClearSong();
			break;

		case WM_PN_PLAYPAUSED :
		{
			INFO("-WM_PN_PLAYPAUSED: wparam: %X, lparam: %X", wparam, lparam);
			MSNMessages msn;
			ZeroMemory(&msn, sizeof(msn));
			msn.msncommand = 1;
			CurrentSong(&msn);

			// Paused
			if (QCDCallbacks->Service(opGetPlayerState, 0, 0, 0) == 3) {
				// Append (paused)
				if (wcslen(msn.title) > TITLE_LEN - 13) {
					msn.title[TITLE_LEN - 13] = 0;
					wcscat_s(msn.title, sizeof(msn.title)/sizeof(WCHAR), L"...");
				}

				wcscat_s(msn.title, sizeof(msn.title)/sizeof(WCHAR), L" (paused)");
			}

			SendToMSN(&msn);

			break;
		}
	}
	
	return CallWindowProc(QCDProc, hwnd, msg, wparam, lparam);
}

//-----------------------------------------------------------------------------
