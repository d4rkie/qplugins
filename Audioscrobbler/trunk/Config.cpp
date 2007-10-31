// Config.cpp
//
//////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include <stdio.h>

#include "md5.h"
#include "AudioscrobblerDLL.h"
#include "Audioscrobbler.h"
#include "Config.h"

//////////////////////////////////////////////////////////////////////
static const _TCHAR  INI_SECTION[]   = _T("Audioscrobbler");
static const char    INI_SECTION_A[] = "Audioscrobbler";

//////////////////////////////////////////////////////////////////////

void LoadSettings()
{
	int nRet = 0;
	WCHAR szBuffer[MAX_PATH] = {0};
	QString strIni;

	QMPCallbacks.Service(opGetPluginSettingsFile, szBuffer, sizeof(szBuffer), 0);
	strIni.SetUnicode(szBuffer);

	Settings.logMode   = (LogMode)GetPrivateProfileInt(INI_SECTION, _T("LogMode"), LOG_NONE, strIni);
	Settings.bFirstRun = GetPrivateProfileInt(INI_SECTION, _T("FirstRun"), 1, strIni);
	
	GetPrivateProfileString(INI_SECTION, _T("Username"), NULL, szBuffer, NUMOFTCHARS(szBuffer), strIni);
	Settings.strUsername.SetUnicode(szBuffer);
	
	ZeroMemory(Settings.strPassword, sizeof(Settings.strPassword));
	nRet = GetPrivateProfileStringA(INI_SECTION_A, "Password", NULL, (char*)szBuffer, NUMOFTCHARS(szBuffer), strIni.GetMultiByte());
	if (nRet == 32) {
		strcpy_s(Settings.strPassword, sizeof(Settings.strPassword), (char*)szBuffer);
		Settings.strPassword[32] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////

void SaveSettings()
{
	WCHAR szBuffer[MAX_PATH] = {0};
	QString strIni;

	QMPCallbacks.Service(opGetPluginSettingsFile, szBuffer, sizeof(szBuffer), 0);
	strIni.SetUnicode(szBuffer);

	_stprintf_s(szBuffer, NUMOFTCHARS(szBuffer), _T("%d"), Settings.logMode);
	WritePrivateProfileString(INI_SECTION, _T("LogMode"), szBuffer, strIni);

	_stprintf_s(szBuffer, NUMOFTCHARS(szBuffer), _T("%d"), Settings.bFirstRun);
	WritePrivateProfileString(INI_SECTION, _T("FirstRun"), szBuffer, strIni);	

	WritePrivateProfileString(INI_SECTION, _T("Username"), Settings.strUsername, strIni);

	// Store password
	Settings.strPassword[32] = NULL;
	WritePrivateProfileStringA(INI_SECTION_A, "Password", Settings.strPassword, strIni.GetMultiByte());
}

//////////////////////////////////////////////////////////////////////

void CreateConfigDlg(HINSTANCE hInstance, HWND hAppHwnd)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hAppHwnd, ConfigDlgProc);
}

//////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL g_bPasswordChanged = FALSE;
	BOOL bReturn = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		SendMessage(GetDlgItem(hwndDlg, IDC_CONFIG_USERNAME), EM_LIMITTEXT, 64, 0);
		SendMessage(GetDlgItem(hwndDlg, IDC_CONFIG_PASSWORD), EM_LIMITTEXT, 64, 0);

		SetDlgItemText(hwndDlg, IDC_CONFIG_USERNAME, Settings.strUsername);
		if (Settings.strPassword[0])
			SetDlgItemText(hwndDlg, IDC_CONFIG_PASSWORD, _T("00000000"));
		g_bPasswordChanged = FALSE;

		SendDlgItemMessage(hwndDlg, IDC_CONFIG_DEBUG, CB_ADDSTRING, 0, (LPARAM)_T("None"));
		SendDlgItemMessage(hwndDlg, IDC_CONFIG_DEBUG, CB_ADDSTRING, 0, (LPARAM)_T("Debug console"));
		SendDlgItemMessage(hwndDlg, IDC_CONFIG_DEBUG, CB_ADDSTRING, 0, (LPARAM)_T("File"));
		SendDlgItemMessage(hwndDlg, IDC_CONFIG_DEBUG, CB_SETCURSEL, (WPARAM)Settings.logMode, 0);

		// Get preferences placement and place accordingly
		HWND hWndPref = (HWND)QMPCallbacks.Service(opGetPropertiesWnd, NULL, 0, 0);
		if (hWndPref) {
			WINDOWPLACEMENT DlgPos, PrefPos;
			DlgPos.length  = sizeof(WINDOWPLACEMENT);
			PrefPos.length = sizeof(WINDOWPLACEMENT);

			GetWindowPlacement(hwndDlg, &DlgPos);
			GetWindowPlacement(hWndPref, &PrefPos);
			int nWidth  = DlgPos.rcNormalPosition.right - DlgPos.rcNormalPosition.left;
			int nHeight = DlgPos.rcNormalPosition.bottom - DlgPos.rcNormalPosition.top;
			DlgPos.rcNormalPosition.left   = PrefPos.rcNormalPosition.left + 220;
			DlgPos.rcNormalPosition.top    = PrefPos.rcNormalPosition.top + 60;
			DlgPos.rcNormalPosition.right  = DlgPos.rcNormalPosition.left + nWidth;
			DlgPos.rcNormalPosition.bottom = DlgPos.rcNormalPosition.top + nHeight;
			
			SetWindowPlacement(hwndDlg, &DlgPos);
		}

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
		{
			_TCHAR szBuffer[128];

			// Validate and save
			//Settings.bShowWindowOnStart = IsDlgButtonChecked(hwndDlg, IDC_SHOW_ON_START);

			GetDlgItemText(hwndDlg, IDC_CONFIG_USERNAME, szBuffer, NUMOFTCHARS(szBuffer));
			Settings.strUsername.SetTStr(szBuffer);
			
			if (g_bPasswordChanged) {
				char szMd5[48];
				GetDlgItemTextA(hwndDlg, IDC_CONFIG_PASSWORD, (char*)szBuffer, NUMOFTCHARS(szBuffer));				
				md5_32(szMd5, (const BYTE*)szBuffer);
				for (int i = 0; i < 32; i++)
					Settings.strPassword[i] = szMd5[i];
				Settings.strPassword[32] = NULL;
			}

			LRESULT nSelected = SendDlgItemMessage(hwndDlg, IDC_CONFIG_DEBUG, CB_GETCURSEL, 0, 0);
			if (nSelected == CB_ERR)
				nSelected = 0;
			Settings.logMode = (LogMode)nSelected;

			g_bPasswordChanged = FALSE;
			
			PostThreadMessage(g_nASThreadId, AS_MSG_SETTINGS_CHANGED, 0, 0);

			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;
		}

		case IDCANCEL :
			g_bPasswordChanged = FALSE;
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		default :
			switch (LOWORD(wParam))
			{
			case IDC_CONFIG_PASSWORD :
				if (HIWORD(wParam) == EN_CHANGE)
					g_bPasswordChanged = TRUE;
				break;
			} // switch (LOWORD(wParam)
			
			break; // defauklt

		} // switch (wParam)
		break;
	} // WM_COMMAND
	
	//case WM_NOTIFY :
	//{
		//if (wParam == IDC_
		/*if (wParam == IDC_CONFIG_DELAY_SPIN) {
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
		}*/
			
	//	break;
	//}
	} // switch (uMsg)

	return bReturn;
}