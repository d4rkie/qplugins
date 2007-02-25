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

#define PLUGIN_NAME "Snarl v1.2"

#define UNICODE 1
#define _UNICODE 1

#include <TCHAR.H>
#include <QString.h>

#include <IQCDTagInfo.h>

#include "Config.h"
#include "QCDGeneralDLL.h"



HINSTANCE        hInstance  = NULL;
HWND             hwndPlayer = NULL;
QCDModInitGen2   QCDCallbacks;
WNDPROC          QCDProc    = NULL;
SnarlInterface*  snarl      = NULL;
Settings         settings;

QString          g_strDefaultIcon;
int		         g_nMsgId = 0;

static const WCHAR INI_SECTION[] = L"Snarl";
static const int WM_REGISTER_MSG = 61091;

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
	QCDCallbacks.size				= sizeof(QCDModInitGen2);
	QCDCallbacks.version			= PLUGIN_API_VERSION_NTUTF8;

	QCDCallbacks.toModule.Initialize= Initialize;
	QCDCallbacks.toModule.ShutDown	= ShutDown;
	QCDCallbacks.toModule.About		= About;
	QCDCallbacks.toModule.Configure	= Configure;

	return &QCDCallbacks;
}

//----------------------------------------------------------------------------

int Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = PLUGIN_NAME;

	hwndPlayer = (HWND)QCDCallbacks.Service(opGetParentWnd, 0, 0, 0);
	LoadSettings();
	snarl = new SnarlInterface();
	snarl->snRegisterConfig(hwndPlayer, "Quintessential Media Player", WM_REGISTER_MSG);

	// Subclass the player and listen for WM_PN_? and WM_REGISTER_MSG
	if ((QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDSubProc)) == 0) {
		MessageBox(hwndPlayer, _T("Failed to subclass player!"), _T("Fatal init error!"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

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
	snarl->snRevokeConfig(hwndPlayer);
	delete snarl;
	SaveSettings();
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	HWND hwnd = (HWND)QCDCallbacks.Service(opGetPropertiesWnd, NULL, 0, 0);
	if (!hwnd)
		hwnd = hwndPlayer;
	CConfig dlg(hInstance, hwnd);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	QString str;
	str.SetMultiByte(PLUGIN_NAME);
	str += _T("\n\nPlug-in by:\nToke Noer (toke@noer.it)");
	MessageBox(hwndPlayer, str , _T("About"), MB_OK | MB_ICONINFORMATION);
}

//-----------------------------------------------------------------------------

void LoadSettings()
{
	WCHAR strIni[MAX_PATH];
	WCHAR strTmp[MAX_PATH];
	QCDCallbacks.Service(opGetPluginSettingsFile, strTmp, MAX_PATH*sizeof(WCHAR), 0); // Returns UTF8
	QCDCallbacks.Service(opUTF8toUCS2, strTmp, (long)strIni, MAX_PATH);

	settings.nTimeout           = GetPrivateProfileIntW(INI_SECTION, L"nTimeout", 7, strIni);
	settings.bCascade           = GetPrivateProfileIntW(INI_SECTION, L"bCascade", 0, strIni);
	settings.bHeadline_wrap      = GetPrivateProfileIntW(INI_SECTION, L"bHeadline_Wrap", 0, strIni);
	//settings.bText1_wrap         = GetPrivateProfileIntW(INI_SECTION, L"bText1_Wrap", 1, strIni);
	//settings.bText2_wrap         = GetPrivateProfileIntW(INI_SECTION, L"bText2_Wrap", 0, strIni);
	settings.Headline_ServiceOp = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bHeadline_Op", opGetArtistName, strIni);
	settings.Text1_ServiceOp    = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bText1_Op", opGetTrackName, strIni);
	settings.Text2_ServiceOp    = (PluginServiceOp)GetPrivateProfileIntW(INI_SECTION, L"bText2_Op", opGetDiscName, strIni);

	// Get icon path
	char strTemp[MAX_PATH];
	WCHAR strFolder[MAX_PATH];
	QCDCallbacks.Service(opGetPluginFolder, strTemp, MAX_PATH, 0);
	QCDCallbacks.Service(opUTF8toUCS2, strTmp, (long)strFolder, MAX_PATH);

	g_strDefaultIcon = strFolder;
	g_strDefaultIcon.append(L"\\snarl.png");
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

	GetIcon(nTrack, strIcon);
	//GetIcon2(nTrack, strIcon);

	// Hide if not cascade
	if (!settings.bCascade && snarl->snIsMessageVisible(g_nMsgId))
		snarl->snHideMessage(g_nMsgId);
	g_nMsgId = snarl->snShowMessage(strHeadline, strText1, settings.nTimeout, (strIcon[0] != '\0') ? strIcon : g_strDefaultIcon.GetUTF8(), 0, 0);
}

//-----------------------------------------------------------------------------

void GetIcon2(long nIndex, LPSTR strIcon)
{
	/*CHAR strUTF8[MAX_PATH];

	QCDCallbacks.Service(opGetPlaylistFile, strUTF8, MAX_PATH, nIndex);
	IQCDTagInfo* pTagInfo = (IQCDTagInfo*)QCDCallbacks.Service(opGetIQCDTagInfo, strUTF8, 0, 0);
	if (pTagInfo)
	{
		WCHAR wszName[2048] = {0};
		DWORD nameLen = 2048;
		DWORD dataLen = 1024;
		int nIndex = 0;
		QCD_TAGDATA_TYPE Type;
		BYTE* pData = new BYTE[1024];

		// get value length
		if (!pTagInfo->ReadFromFile(TAG_DEFAULT))
			MessageBox(NULL, L"Failed to ReadFromFile", L"", 0);

		int nCount = pTagInfo->GetFieldCount();

		for (int i = 0; i < nCount; i++)
		{
			nameLen = 2048;
			dataLen = 1024;

			if (pTagInfo->GetTagDataByIndex (i, wszName, &nameLen, &Type, pData, &dataLen))
			{
				MessageBoxW(NULL, wszName, L"", 0);
			}
		}

		delete [] pData;

		/*WCHAR str[64];
		swprintf(str, 64, L"%i", nCount);
		MessageBoxW(NULL, str, L"", 0);*/



		/*if (pTagInfo->GetTagDataByName(QCDTag_Artwork, &Type, pValue, &dataLen, &startIndex))
		{
			MessageBox(hwndPlayer, L"GetTagDataByName", L"", 0);

			if (dataLen > 0)
			{
				/*BYTE* pValue = new BYTE[valueLen];
				if (pValue)
				{
					QCDCallbacks.toPlayer.GetTagDataByIndex(tagHandle, index, NULL, 0, &Type, pValue, &valueLen);
					QTD_STRUCT_ARTWORK* pArtwork = (QTD_STRUCT_ARTWORK*)bValue;
					// ... access artwork
					delete[] pValue;
				}
				MessageBox(hwndPlayer, L"Test", L"", 0);
			}
		}

		pTagInfo->Release();
	}
	_tcscpy(strIcon, _T(""));
	*/
}

void GetIcon(long nIndex, LPSTR strIcon)
{
	char strUTF8[MAX_PATH];

	TCHAR* pPathEnd = NULL;
	TCHAR  szPath[MAX_PATH] = {0};
	QString strFilename;
	QString strArtist;
	QString strAlbum;
	QString strIconFile;

	// Get path
	QCDCallbacks.Service(opGetTrackFile, strUTF8, MAX_PATH, nIndex);
	strFilename.SetUTF8(strUTF8);
	GetFullPathName(strFilename.GetUnicode(), MAX_PATH, szPath, &pPathEnd);
	*pPathEnd = '\0';

	// Get Artist
	QCDCallbacks.Service(opGetArtistName, strUTF8, MAX_PATH, nIndex);
	strArtist.SetUTF8(strUTF8);

	// Get Album
	QCDCallbacks.Service(opGetDiscName, strUTF8, MAX_PATH, nIndex);
	strAlbum.SetUTF8(strUTF8);

	// Create path
	strIconFile.assign(szPath);
	strIconFile.append(strArtist);
	strIconFile.append(_T(" - "));
	strIconFile.append(strAlbum);
	strIconFile.append(_T(" - front.jpg"));

	// Check if the file exist
	HANDLE hFile = CreateFile(strIconFile, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
								NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		strcpy(strIcon, "");
	else
	{
		CloseHandle(hFile);
		strcpy(strIcon, strIconFile.GetUTF8());
	}
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
		/*case WM_REGISTER_MSG :
			MessageBox(0, "test", "", 0);
			Configure(0);
			return 0;
		*/
	}
	
	return CallWindowProc(QCDProc, hwnd, msg, wparam, lparam);
}