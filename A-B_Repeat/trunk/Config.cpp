// Config.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#include <TCHAR.h>
#include <windows.h>
#include <stdio.h>

#include "ABRepeatDLL.h"
#include "Config.h"

//////////////////////////////////////////////////////////////////////
static const _TCHAR INI_SECTION[] = _T("A-B_Repeat");

void GetPluginSettingsFile(_TCHAR* str)
{
#if defined UNICODE
	WCHAR strTmp[MAX_PATH];
	QCDCallbacks->Service(opGetPluginSettingsFile, strTmp, MAX_PATH*2, 0);
	QCDCallbacks->Service(opUTF8toUCS2, strTmp, (long)str, MAX_PATH);
#else
	QCDCallbacks->Service(opGetPluginSettingsFile, str, MAX_PATH, 0);
#endif
}

//////////////////////////////////////////////////////////////////////

void LoadSettings()
{
	_TCHAR strIni[MAX_PATH];
	GetPluginSettingsFile(strIni);

	Settings.bShowWindowOnStart = GetPrivateProfileInt(INI_SECTION, _T("bShowWindowOnStart"), 1, strIni);
}

//////////////////////////////////////////////////////////////////////

void SaveSettings()
{
	_TCHAR buf[32];
	_TCHAR strIni[MAX_PATH];
	GetPluginSettingsFile(strIni);

	_stprintf_s(buf, 32, _T("%d"), Settings.bShowWindowOnStart);
	WritePrivateProfileString(INI_SECTION, _T("bShowWindowOnStart"), buf, strIni);
}

//////////////////////////////////////////////////////////////////////

void CreateConfigDlg(HINSTANCE hInstance, HWND hAppHwnd)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hAppHwnd, ConfigDlgProc);
}

//////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bReturn = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		CheckDlgButton(hwndDlg, IDC_SHOW_ON_START, Settings.bShowWindowOnStart);

		//SendDlgItemMessage(hwndDlg, IDC_CONFIG_DELAY_SPIN, UDM_SETRANGE, 0, MAKELONG(30, 0));
		//SetDlgItemInt(hwndDlg, IDC_CONFIG_DELAY, settings.nDelay / 1000, FALSE);

		bReturn = TRUE;
		break;
	}
	case WM_CLOSE :
	{
		EndDialog(hwndDlg, TRUE);
		bReturn = TRUE;
		break;
	}
	case WM_COMMAND :
	{
		switch (wParam)
		{
		case IDOK :
			//_TCHAR strBuff[32];

			// Validate and save
			Settings.bShowWindowOnStart = IsDlgButtonChecked(hwndDlg, IDC_SHOW_ON_START);

			//if (GetDlgItemText(hwndDlg, IDC_CONFIG_DELAY, strBuff, sizeof(strBuff)/sizeof(_TCHAR)))
			//	settings.nDelay = _ttoi(strBuff) * 1000;

			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		case IDCANCEL :
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		} // switch (wParam)

		break;
	}
	/*case WM_NOTIFY :
	{
		if (wParam == IDC_CONFIG_DELAY_SPIN) {
			_TCHAR strTemp[64];
			NMUPDOWN* nm = (NMUPDOWN*)lParam;

			GetDlgItemText(hwndDlg, IDC_CONFIG_DELAY, strTemp, sizeof(strTemp)/sizeof(_TCHAR));
			settings.nDelay  = _ttoi(strTemp) * 1000;
			settings.nDelay += nm->iDelta * 1000;
			if (settings.nDelay > 45000 && nm->iDelta > 0)
				settings.nDelay = 45000;
			else if (settings.nDelay > 45000 && nm->iDelta < 0)
				settings.nDelay = 0;
			_itot_s(settings.nDelay / 1000, strTemp, sizeof(strTemp)/sizeof(_TCHAR), 10);
			SetDlgItemText(hwndDlg, IDC_CONFIG_DELAY, strTemp);

			bReturn = TRUE;
		}
			
		break;
	}*/
	} // switch (uMsg)

	return bReturn;
}