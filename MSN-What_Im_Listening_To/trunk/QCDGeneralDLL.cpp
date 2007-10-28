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
//    - Find a reliable way to detect encoding. (Through right click on playlist)
//
// Known bugs:
// 
// Fixes since last release:
//    + Miranda IM support
//    + Uses the new QMP entry and interfaces when running under QMP
//    * Small efficiency improvements
//    * Small fix for playing CDs
//    * Removed support for old MSN beta version. (build 572 or simething)
//
//-----------------------------------------------------------------------------

#define UNICODE 1
#define _UNICODE 1

#include <Windows.h>
// #include <stdio.h>
#include <TCHAR.H>
#include <strsafe.h>
#include <debug.h>

//-----------------------------------------------------------------------------

#include "QMPHelperGeneral.h"

#include "QCDGeneralDLL.h"
#include "QBlog.h"
#include "WMP_Reg.h"
#include "Config.h"
#include "About.h"

#pragma warning(disable:4002)		// Disable "too many actual parameters for macro INFO"


// Constants
const static CHAR*   PLUGIN_NAME   =  "MSN \"What I'm Listening To\" v2.6.0";
const static WCHAR*  PLUGIN_NAME_W = L"MSN \"What I'm Listening To\" v2.6.0";

const static LPCTSTR MSN_WINDOWCLASS       = _T("MsnMsgrUIManager");

const static LPCTSTR MIRANDA_WINDOWCLASS   = _T("Miranda.ListeningTo");
const static long    MIRANDA_DW_PROTECTION = 0x8754;

static const WCHAR   INI_SECTION[] = L"MSN-WILT";

static const WORD    TITLE_LEN     = 100;
static const WORD    ALBUM_LEN     = 100;
static const WORD    ARTIST_LEN    = 100;
static const WORD    CONTENTID_LEN = 100;


// Global variables
HINSTANCE          hInstance        = NULL;
HWND               hwndPlayer       = NULL;

QCDService*        QCDCallbacks     = NULL;
QCDModInitGen*     ModInitGen1      = NULL;
QCDModInitGen2*    ModInitGen2      = NULL;

BOOL               g_IsNewApi       = false;

WNDPROC            g_lpOldQMPProc   = NULL;
UINT_PTR           g_nDelayTimerID  = 0;
QString            g_strTrackPlaying;
Settings           settings;

//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		#ifdef _DEBUG
			_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
			// _CrtSetBreakAlloc(59);
		#endif

		DisableThreadLibraryCalls(hInst);
		hInstance = hInst;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
// New QMP entry point
//-----------------------------------------------------------------------------
PLUGIN_API QCDModInitGen2* GENERAL2_DLL_ENTRY_POINT()
{
	ModInitGen2 = new QCDModInitGen2;

	ModInitGen2->size                 = sizeof(QCDModInitGen2);
	ModInitGen2->version              = PLUGIN_API_VERSION_NTUTF8;

	ModInitGen2->toModule.Initialize  = Initialize;
	ModInitGen2->toModule.ShutDown    = ShutDown;
	ModInitGen2->toModule.About       = About;
	ModInitGen2->toModule.Configure   = Configure;

	g_IsNewApi = true;
	return ModInitGen2;
}

//-----------------------------------------------------------------------------
// Old QCD entry point
//-----------------------------------------------------------------------------
PLUGIN_API BOOL GENERALDLL_ENTRY_POINT(QCDModInitGen *ModInit, QCDModInfo *ModInfo)
{
	ModInitGen1 = ModInit;

	ModInitGen1->size                 = sizeof(QCDModInitGen);		// Old API sizeof
	ModInitGen1->version              = PLUGIN_API_VERSION_NTUTF8;

	ModInitGen1->toModule.ShutDown    = ShutDown;
	ModInitGen1->toModule.About       = About;
	ModInitGen1->toModule.Configure   = Configure;

	g_IsNewApi = false;

	Initialize(ModInfo, 0);
	return TRUE;
}

//----------------------------------------------------------------------------

int Initialize(QCDModInfo *modInfo, int flags)
{
	QCDCallbacks = new QCDService;
	if (g_IsNewApi)
		QCDCallbacks->Service = ModInitGen2->Service;
	else
		QCDCallbacks->Service = ModInitGen1->Service;
	modInfo->moduleString = const_cast<char*>(PLUGIN_NAME);

	InitializeHelper(QCDCallbacks->Service);

	// TODO: all your plugin initialization here
	g_nDelayTimerID = 0;
	hwndPlayer    = (HWND)QCDCallbacks->Service(opGetParentWnd, 0, 0, 0);
	g_strTrackPlaying.SetUnicode(L"");

	//nMSNBuild     = RegDB_GetMSNBuild();

	// Subclass the player and listen for WM_PN_?
	if ((g_lpOldQMPProc = (WNDPROC)SetWindowLongPtr(hwndPlayer, GWLP_WNDPROC, (LONG_PTR)QCDSubProc)) == 0){
		MessageBox(hwndPlayer, _T("Failed to subclass player!"), _T("Fatal init error!"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	LoadSettings();
	if (IsPlayerStatus(QMP_PLAYING) && !IsEncoding())
	{
		PlayingSongInfo songInfo;
		// ZeroMemory(&songInfo, sizeof(PlayingSongInfo));
		CurrentSong(&songInfo);
		songInfo.nCommand = 1;

		SendSongInfo(&songInfo);
	}

	return TRUE; // return TRUE for successful initialization
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	// Remove subclassing
	if (!SetWindowLongPtr(hwndPlayer, GWLP_WNDPROC, (LONG_PTR)g_lpOldQMPProc))
		MessageBox(hwndPlayer, L"Failed to remove window hook. Restart the player!", PLUGIN_NAME_W, 0);

	if (g_nDelayTimerID)
		KillTimer(NULL, g_nDelayTimerID);

	ClearSong();
	QBlog_CleanUpRegDB();

	SaveSettings();

	delete QCDCallbacks;
	if (g_IsNewApi)
		delete ModInitGen2;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	HWND hwnd = (HWND)QCDCallbacks->Service(opGetPropertiesWnd, NULL, 0, 0);

	CConfig dlg(hInstance, hwnd != NULL ? hwnd : hwndPlayer);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	HWND hwnd = (HWND)QCDCallbacks->Service(opGetPropertiesWnd, NULL, 0, 0);

	CAbout dlg(hInstance, hwnd != NULL ? hwnd : hwndPlayer);
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

	settings.bTitle      = TRUE;
	settings.bArtist     = GetPrivateProfileInt(INI_SECTION, L"bArtist", 1, strIni);
	settings.bAlbum      = GetPrivateProfileInt(INI_SECTION, L"bAlbum",  0, strIni);
	settings.bVideo      = GetPrivateProfileInt(INI_SECTION, L"bVideo",  0, strIni);
	settings.bWMPIsFaked = GetPrivateProfileInt(INI_SECTION, L"bWMPIsFaked", 0, strIni);
	settings.nDelay      = GetPrivateProfileInt(INI_SECTION, L"nDelay",  0, strIni);

	settings.bDebug      = GetPrivateProfileInt(INI_SECTION, L"bDebug",  0, strIni);

	bFirstRun = GetPrivateProfileInt(INI_SECTION, L"bFirstRun", 1, strIni);
	if (bFirstRun)
	{
		if (RegDB_GetWMPVersion() < 9)
		{
			if (IDYES == MessageBox(hwndPlayer, 
					L"The \"MSN - What Im Listening To\" QMP plug-in has detected that WMP (Windows Media Player)\n" \
					L"is not installed on your computer or you are running an old version which is not supported.\n\n" \
					L"In order to turn on the \"What Im Listening To\" feature in MSN you have to either:\n" \
					L"  1) Install Windows Media Player\n"\
					L"  2) Let this plug-in fix it without need of WMP\n\n" \
					L"Do you want to let the plug-in fix the problem now?\n" \
					L"(This can be undone from the configuration dialog of this plug-in.)\n\n" \
					L"Note: Users of non official clients, for example Miranda, might not need this fix!", 
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
	
	StringCchPrintf(buf, 32, L"%d", settings.bDebug);      WritePrivateProfileString(INI_SECTION, L"bDebug",  buf, strIni);
	StringCchPrintf(buf, 32, L"%d", settings.bArtist);     WritePrivateProfileString(INI_SECTION, L"bArtist", buf, strIni);
	StringCchPrintf(buf, 32, L"%d", settings.bAlbum);      WritePrivateProfileString(INI_SECTION, L"bAlbum",  buf, strIni);
	StringCchPrintf(buf, 32, L"%d", settings.bVideo);      WritePrivateProfileString(INI_SECTION, L"bVideo",  buf, strIni);
	StringCchPrintf(buf, 32, L"%d", settings.bWMPIsFaked); WritePrivateProfileString(INI_SECTION, L"bWMPIsFaked",  buf, strIni);
	StringCchPrintf(buf, 32, L"%u", settings.nDelay);      WritePrivateProfileString(INI_SECTION, L"nDelay",  buf, strIni);
}

//-----------------------------------------------------------------------------
// Song related functions
//-----------------------------------------------------------------------------
void SendSongInfo(PlayingSongInfo* pSongInfo)
{
	INFO("SendSongInfo()");

	// Send empty if not showing video info
	if (settings.bVideo == FALSE && IsVideo())
		pSongInfo->nCommand = 0;

	if (!SendToMiranda(pSongInfo))
		SendToMSN(pSongInfo);
}

//-----------------------------------------------------------------------------

BOOL SendToMiranda(PlayingSongInfo* pSongInfo)
{
	BOOL bReturn = FALSE;
	HWND hwnd = NULL;

	hwnd = FindWindowEx(NULL, NULL, MIRANDA_WINDOWCLASS, NULL);
	if (hwnd == NULL)
		return FALSE;

   // Sent to Miranda: 1\0QMP\0Music\0\0\0\0-858993460\0\0\00\0\0\0
	// Original string: L"<Status>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0<Radio Station>\\0\\0"
	static const WCHAR* MirandaMusicStr = L"%d\\0QMP\\0%s\\0%s\\0%s\\0%s\\0%d\\0%s\\0%s\\0%d\\0%s\\0\\0";

	WCHAR strBuffer[4092];
	ZeroMemory(&strBuffer, sizeof(strBuffer));

	if (pSongInfo->nCommand == 1)
	{
		StringCbPrintf(strBuffer, sizeof(strBuffer), MirandaMusicStr, 
			pSongInfo->nCommand, pSongInfo->strType.GetUnicode(), pSongInfo->strTitle.GetUnicode(), 
			pSongInfo->strArtist.GetUnicode(), pSongInfo->strAlbum.GetUnicode(), pSongInfo->nTrackNumber,
			pSongInfo->strYear.GetUnicode(), pSongInfo->strGenre.GetUnicode(), pSongInfo->nLength, pSongInfo->strRadioStationName.GetUnicode()
		);
	}
	else
		StringCbCopy(strBuffer, sizeof(strBuffer), L"0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0");

	// Create copy struct message
	COPYDATASTRUCT data;
	data.dwData = MIRANDA_DW_PROTECTION;
	data.lpData = &strBuffer;
	StringCbLength(strBuffer, sizeof(strBuffer), (size_t*)&(data.cbData));
	data.cbData += 2; // Size in bytes incl. last wide NULL

	do
	{
		SendMessage(hwnd, WM_COPYDATA, NULL, (LPARAM)&data);
		bReturn = TRUE;

		#ifndef _DEBUG
			if (settings.bDebug) {
		#endif
			OutputDebugStringW(L"Sent to Miranda: ");
			OutputDebugStringW(strBuffer);
			OutputDebugStringW(L"\n");
		#ifndef _DEBUG
			}
		#endif
	} while (hwnd = FindWindowEx(NULL, hwnd, MIRANDA_WINDOWCLASS, NULL));

	return bReturn;
}

//-----------------------------------------------------------------------------

BOOL SendToMSN(PlayingSongInfo* pSongInfo)
{
	BOOL bReturn = FALSE;
	HWND hwnd = NULL;

	hwnd = FindWindowEx(NULL, NULL, MSN_WINDOWCLASS, NULL);
	if (hwnd == NULL)
		return FALSE;


	// Original string: "WMP\0Music\0[status]\0{0} - {1}\0[Title]\0[Artist]\0[Album]\0[contentid]\0"
	static const WCHAR* MSNMusicStr = L"QMP\\0Music\\0%d\\0%s\\0%s\\0%s\\0%s\\0%s\\0";
	static const size_t STR_CCH = TITLE_LEN + ALBUM_LEN + ARTIST_LEN + CONTENTID_LEN + 4 + 32;
	
	WCHAR strMSNFormat[32] = L"{0}";
	WCHAR strBuffer[STR_CCH];
	ZeroMemory(&strBuffer, sizeof(strBuffer));

	if (pSongInfo->nCommand == 1)
	{
		// Format string
		if (settings.bArtist && pSongInfo->strArtist.Length() > 0)
			StringCchCat(strMSNFormat, 64, L" - {1}");
		if (settings.bAlbum && pSongInfo->strAlbum.Length() > 0)
			StringCchCat(strMSNFormat, 64, L" - {2}");

		StringCchPrintf(strBuffer, STR_CCH, MSNMusicStr, pSongInfo->nCommand, strMSNFormat, pSongInfo->strTitle.GetUnicode(), pSongInfo->strArtist.GetUnicode(), pSongInfo->strAlbum.GetUnicode(), pSongInfo->strWmContentId.GetUnicode());
	}
	else
		StringCchCopy(strBuffer, STR_CCH, L"QMP\\0Music\\00\\0{0}\\0\\0\\0\\0\\0");

	// Create copy struct message
	COPYDATASTRUCT msndata;
	msndata.dwData = 0x547;
	msndata.lpData = &strBuffer;
	StringCbLength(strBuffer, STR_CCH*sizeof(WCHAR), (size_t*)&(msndata.cbData));
	msndata.cbData += 2; // Size in bytes incl. last wide NULL

	// We already have the first hwnd to send to and have exited on hwnd == null
	do
	{
		SendMessage(hwnd, WM_COPYDATA, NULL, (LPARAM)&msndata);
		bReturn = TRUE;

		#ifndef _DEBUG
			if (settings.bDebug) {
		#endif
			OutputDebugStringW(L"Sent to MSN: ");
			OutputDebugStringW(strBuffer);
			OutputDebugStringW(L"\n");
		#ifndef _DEBUG
			}
		#endif
	} while (hwnd = FindWindowEx(NULL, hwnd, MSN_WINDOWCLASS, NULL));

	return bReturn;
}

//-----------------------------------------------------------------------------
// Fill song information
//		Notice: This functions sets all variables of the song struct !
void CurrentSong(PlayingSongInfo* pSongInfo)
{
	INFO("CurrentSong()");

	pSongInfo->nCommand = 0;

	const static int BUF_SIZE = 1024;
	long nDataSize = 0;
	char strUTF8[256] = {0};
	WCHAR strUCS2[BUF_SIZE] = {0};

	///////////////////////////////////////////////////////////////////////////
	// Fill information shared between the two retrieval methods
	///////////////////////////////////////////////////////////////////////////
	
	if (g_IsNewApi)
	{
		long nReturn = QCDCallbacks->Service(opGetMediaType, 0, -1, 0);
		if (nReturn == DIGITAL_AUDIOFILE_MEDIA || nReturn == CD_AUDIO_MEDIA)
			pSongInfo->strType.SetUnicode(L"Music");
		else if (nReturn == DIGITAL_AUDIOSTREAM_MEDIA)
			pSongInfo->strType.SetUnicode(L"Radio");
		else if (nReturn == DIGITAL_VIDEO_MEDIA)
			pSongInfo->strType.SetUnicode(L"Video");
		else
			pSongInfo->strType.SetUnicode(L"Unknown");
	}
	else
		pSongInfo->strType.SetUnicode(L"Music");

	pSongInfo->nLength = QCDCallbacks->Service(opGetTrackLength, NULL, -1, 0);
	
	// Initialize variables we are not sure to be set - Only important for scalar values
	pSongInfo->nTrackNumber = 0;
	//pSongInfo->strRadioStationName.SetUnicode(L"");
	//pSongInfo->strWmContentId.SetUnicode(L"");


	///////////////////////////////////////////////////////////////////////////
	// Use new interface method for QMP and old version for QCD
	///////////////////////////////////////////////////////////////////////////

	if (g_IsNewApi)
	{
		IQCDMediaInfo* info = (IQCDMediaInfo*)QCDCallbacks->Service(opGetIQCDMediaInfo, (void*)g_strTrackPlaying.GetUTF8(), 0, 0);
		if (info)
		{
			info->LoadFullData();

			// Artist
			nDataSize = BUF_SIZE;
			info->GetInfoByName(QCDInfo_ArtistTrack, strUCS2, &nDataSize);
			pSongInfo->strArtist.SetUnicode(strUCS2);

			// Title
			nDataSize = BUF_SIZE;
			info->GetInfoByName(QCDInfo_TitleTrack, strUCS2, &nDataSize);
			pSongInfo->strTitle.SetUnicode(strUCS2);

			// Album
			nDataSize = BUF_SIZE;
			info->GetInfoByName(QCDInfo_TitleAlbum, strUCS2, &nDataSize);
			pSongInfo->strAlbum.SetUnicode(strUCS2);

			// Tracknumber
			WCHAR szTrackNr[64] = {0};
			nDataSize = BUF_SIZE;
			if ( info->GetInfoByName(QCDInfo_TrackNumber, strUCS2, &nDataSize) ) {
				WCHAR* pData = strUCS2;
				int i = 0;
				for ( ; *pData && *pData != '/'; i++)
					strUCS2[i] = *pData++;
				strUCS2[i] = NULL;

				pSongInfo->nTrackNumber = _wtoi(strUCS2);
			}

			// Year
			nDataSize = BUF_SIZE;
			info->GetInfoByName(QCDInfo_YearAlbum, strUCS2, &nDataSize);
			pSongInfo->strYear.SetUnicode(strUCS2);

			// Genre
			nDataSize = BUF_SIZE;
			info->GetInfoByName(QCDInfo_GenreTrack, strUCS2, &nDataSize);
			pSongInfo->strGenre.SetUnicode(strUCS2);

			if (IsStream()) 
			{
				nDataSize = BUF_SIZE;
				info->GetInfoByName(QCDInfo_TitleAlbum, strUCS2, &nDataSize);
				pSongInfo->strRadioStationName.SetUnicode(strUCS2);
			}

			/////////////////////////////////////////////////////////////////////
			info->Release();
		}
	}
	else
	{
		// Title, artist and album
		QCDCallbacks->Service(opGetTrackName, strUTF8, sizeof(strUTF8), -1);
		pSongInfo->strTitle.SetUTF8(strUTF8);

		QCDCallbacks->Service(opGetArtistName, strUTF8, sizeof(strUTF8), -1);
		pSongInfo->strArtist.SetUTF8(strUTF8);

		QCDCallbacks->Service(opGetAlbumName, strUTF8, sizeof(strUTF8), -1);
		pSongInfo->strAlbum.SetUTF8(strUTF8);

		pSongInfo->nTrackNumber = QCDCallbacks->Service(opGetTrackNum, NULL, -1, 0);
	}
}

//-----------------------------------------------------------------------------

void UpdateSong(BOOL bSendBlogInfo)
{
	INFO("UpdateSong()");

	if (IsEncoding())
		return;

	char strTemp[MAX_PATH*2] = {0};
	QCDCallbacks->Service(opGetTrackFile, strTemp, sizeof(strTemp), -1);
	g_strTrackPlaying.SetUTF8(strTemp);

	PlayingSongInfo songInfo;

	CurrentSong(&songInfo);
	songInfo.nCommand = 1;

	if (bSendBlogInfo)
		QBlog_InsertInRegDB(&songInfo);

	SendSongInfo(&songInfo);
}

//-----------------------------------------------------------------------------

void ClearSong()
{
	INFO("ClearSong()");

	g_strTrackPlaying.SetUnicode(L"");

	PlayingSongInfo songInfo;
	songInfo.nCommand = 0;

	SendSongInfo(&songInfo);

	QBlog_CleanUpRegDB();
}

//-----------------------------------------------------------------------------

void PlayPaused()
{
	INFO("PlayPaused()");

	if (IsEncoding())
		return;

	PlayingSongInfo songInfo;

	CurrentSong(&songInfo);
	songInfo.nCommand = 1;

	// Append paused, if paused
	if (IsPlayerStatus(QMP_PAUSED))
	{ 
		if (songInfo.strTitle.Length() > TITLE_LEN - 13)
		{
			std::wstring strTmp = songInfo.strTitle.GetUnicode();

			songInfo.strTitle.SetUnicode(strTmp.substr(0, TITLE_LEN - 13).c_str());
			songInfo.strTitle.AppendUnicode(L"...");
		}

		songInfo.strTitle.AppendUnicode(L" (paused)");
	}

	SendSongInfo(&songInfo);
}

void StartTimer(UINT nForced /* = 0 */)
{
	if (g_nDelayTimerID) {
		KillTimer(NULL, g_nDelayTimerID);
		g_nDelayTimerID = 0;
	}

	// Don't wait before setting QBlog information, unless it is a CD
	if (nForced == 0 && !IsEncoding())
	{
		char strTemp[MAX_PATH*2] = {0};
		QCDCallbacks->Service(opGetTrackFile, strTemp, sizeof(strTemp), -1);
		g_strTrackPlaying.SetUTF8(strTemp);

		PlayingSongInfo songInfo;
		CurrentSong(&songInfo);
		songInfo.nCommand = 1;
		QBlog_InsertInRegDB(&songInfo);
	}

	if (settings.nDelay > 0 || nForced > 0)
		g_nDelayTimerID = SetTimer(NULL, 0, (settings.nDelay >= nForced) ? settings.nDelay : nForced, DelayTimerProc);
	else
		UpdateSong(FALSE);
}

//-----------------------------------------------------------------------------

void CALLBACK DelayTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	KillTimer(NULL, g_nDelayTimerID);

	UpdateSong(TRUE);
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{
	switch (msg)
	{
		/*
		case WM_PN_ENCODESTARTED :
			INFO("-WM_PN_ENCODESTARTED: wparam: %X, lparam: %X", wparam, lparam);
			break;
		case WM_PN_ENCODEPROGRESS :
			INFO("-WM_PN_ENCODEPROGRESS: wparam: %X, lparam: %X", wparam, lparam);
			break;
		*/

		// Set info
		case WM_PN_INFOCHANGED :
		{
			INFO("-WM_PN_INFOCHANGED: wparam: %X, lparam: %X", wparam, lparam);

			// Check that it's not a cd/dvd that was inserted
			if (lparam <= 255)
				break;
			if (IsPlayerStatus(QMP_STOPPED))
				break;
			// Only fall through if info has changed for the current playing track
			if (CompareStringA(LOCALE_USER_DEFAULT, 0, (LPCSTR)lparam, -1, g_strTrackPlaying.GetUTF8(), -1) != CSTR_EQUAL)
				break;
		}
		// Fall through
		case WM_PN_TRACKCHANGED :
			INFO("-WM_PN_TRACKCHANGED: wparam: %X, lparam: %X", wparam, lparam);

			StartTimer(100);	// Workaround for CD track change
			break;
		
		case WM_PN_PLAYSTARTED :
			INFO("-WM_PN_PLAYSTARTED: wparam: %X, lparam: %X", wparam, lparam);

			StartTimer();
			break;

		case WM_PN_PLAYSTOPPED :
			INFO("-WM_PN_PLAYSTOPPED: wparam: %X, lparam: %X", wparam, lparam);
			// Fallthrough
		case WM_PN_PLAYDONE :
			INFO("-WM_PN_PLAYDONE: wparam: %X, lparam: %X", wparam, lparam);
			
			ClearSong();
			break;

		case WM_PN_PLAYPAUSED :
			INFO("-WM_PN_PLAYPAUSED: wparam: %X, lparam: %X", wparam, lparam);
			
			PlayPaused();
			break;

	} // switch
	
	return CallWindowProc(g_lpOldQMPProc, hwnd, msg, wparam, lparam);
}

//-----------------------------------------------------------------------------


/*
				int i = 0;
				long nNameLen = 1024;
				long nValueLen = 1024;
				WCHAR szName[1024];
				WCHAR szValue[1024];
				while ( info->GetInfoByIndex(i++, szName, &nNameLen, szValue, &nValueLen) )
				{
					OutputDebugString(L" : ");
					OutputDebugString(szName);
					OutputDebugString(L"=");
					OutputDebugString(szValue);
					OutputDebugString(L"\n");

					nNameLen = 1024;
					nValueLen = 1024;
				}
*/