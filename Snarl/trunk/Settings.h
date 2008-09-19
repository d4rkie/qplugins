#ifndef Settings_H_C6D9A95E07D84949832B0E02F7C094A2
#define Settings_H_C6D9A95E07D84949832B0E02F7C094A2

#include <deque>
#include <qcdtagdata.h>
#include "QCDGeneralDLL.h"

static const WCHAR INI_SECTION[] = L"Snarl";


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
	Settings.strCoverArtRoot.SetUnicode(strTmp);
	GetPrivateProfileStringW(INI_SECTION, L"CoverArtTemplate", L"%A - %D - Front.%E", strTmp, MAX_PATH, strIni);
	Settings.strCoverArtTemplate.SetUnicode(strTmp);

	// Get icon path
	WCHAR szPluginfolder[MAX_PATH];
	Service(opGetPluginFolder, szPluginfolder, MAX_PATH*sizeof(WCHAR), 0);
	g_strDefaultIcon = szPluginfolder;
	g_strDefaultIcon.AppendUnicode(L"\\snarl.png");

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


#endif // Settings_H_C6D9A95E07D84949832B0E02F7C094A2