// Config.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>
#include <TCHAR.h>
#include <QString.h>
#include "QCDGeneralDLL.h"
#include "Config.h"

#include "SnarlInterface.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig::CConfig(HINSTANCE hInstance, HWND hAppHwnd)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hAppHwnd, DlgProc);
}

CConfig::~CConfig()
{

}

//////////////////////////////////////////////////////////////////////

const static TCHAR* strComboBox[4] = {_T("Title"), _T("Artist"), _T("Album"), _T("Nothing")};
const static PluginServiceOp ServiceOps[4] = {opGetTrackName, opGetArtistName, opGetDiscName, (PluginServiceOp)0};

BOOL CALLBACK CConfig::DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bReturn = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		// Fill combobox with standard stuff
		HWND hwnd;
		HWND hwndCombos[3];
		hwndCombos[0] = GetDlgItem(hwndDlg, IDC_HEADLINE_COMBO);
		hwndCombos[1] = GetDlgItem(hwndDlg, IDC_TEXT1_COMBO);
		hwndCombos[2] = GetDlgItem(hwndDlg, IDC_TEXT2_COMBO);
		for (int i = 0; i < 3; i++)
		{
			SendMessage(hwndCombos[i], CB_ADDSTRING, 0, (LPARAM)strComboBox[0]);
			SendMessage(hwndCombos[i], CB_ADDSTRING, 0, (LPARAM)strComboBox[1]);
			SendMessage(hwndCombos[i], CB_ADDSTRING, 0, (LPARAM)strComboBox[2]);
		}
		SendMessage(hwndCombos[2], CB_ADDSTRING, 0, (LPARAM)strComboBox[3]);

		// Setup via variables
		SendDlgItemMessage(hwndDlg, IDC_TIME_SPIN, UDM_SETRANGE, 0, MAKELONG(30, 0));
		SetDlgItemInt(hwndDlg, IDC_TIME, settings.nTimeout, FALSE);
		
		for (int i=0; i<4; i++) {
			if (settings.Headline_ServiceOp == ServiceOps[i])
				SendMessage(hwndCombos[0], CB_SETCURSEL, i, 0);
			if (settings.Text1_ServiceOp == ServiceOps[i])
				SendMessage(hwndCombos[1], CB_SETCURSEL, i, 0);
			if (settings.Text2_ServiceOp == ServiceOps[i])
				SendMessage(hwndCombos[2], CB_SETCURSEL, i, 0);
		}
		
		CheckDlgButton(hwndDlg, IDC_CASCADE_POPUPS, settings.bCascade);
		CheckDlgButton(hwndDlg, IDC_HEADLINE_WRAP,  settings.bHeadline_wrap);
		//CheckDlgButton(hwndDlg, IDC_TEXT1_WRAP,     settings.bText1_wrap);
		//CheckDlgButton(hwndDlg, IDC_TEXT2_WRAP,     settings.bText2_wrap);

		// Snarl version
		WORD nHi, nLo;
		TCHAR str[64];
		snarl->snGetVersion(&nHi, &nLo);
		_stprintf_s(str, 64, _T("%u.%u"), nHi, nLo);
		SetDlgItemText(hwndDlg, IDC_SNARL_VERSION, str);

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
			_TCHAR strBuff[32];
			HWND hwndCombos[3];
			hwndCombos[0] = GetDlgItem(hwndDlg, IDC_HEADLINE_COMBO);
			hwndCombos[1] = GetDlgItem(hwndDlg, IDC_TEXT1_COMBO);
			hwndCombos[2] = GetDlgItem(hwndDlg, IDC_TEXT2_COMBO);

			// Validate and save
			settings.bCascade       = IsDlgButtonChecked(hwndDlg, IDC_CASCADE_POPUPS);
			settings.bHeadline_wrap = IsDlgButtonChecked(hwndDlg, IDC_HEADLINE_WRAP);
			//settings.bText1_wrap    = IsDlgButtonChecked(hwndDlg, IDC_TEXT1_WRAP);
			//settings.bText2_wrap    = IsDlgButtonChecked(hwndDlg, IDC_TEXT2_WRAP);
			
			settings.Headline_ServiceOp = ServiceOps[SendMessage(hwndCombos[0], CB_GETCURSEL, 0, 0)];
			settings.Text1_ServiceOp    = ServiceOps[SendMessage(hwndCombos[1], CB_GETCURSEL, 0, 0)];
			settings.Text2_ServiceOp    = ServiceOps[SendMessage(hwndCombos[2], CB_GETCURSEL, 0, 0)];

			if (GetDlgItemText(hwndDlg, IDC_TIME, strBuff, sizeof(strBuff)/sizeof(_TCHAR)))
				settings.nTimeout = _ttoi(strBuff);

			if (IsPlaying())
				DisplaySongInfo();

			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		} // switch (wParam)

		break;
	}
	case WM_NOTIFY :
	{
		if (wParam == IDC_TIME_SPIN) {
			_TCHAR strTemp[64];
			NMUPDOWN* nm = (NMUPDOWN*)lParam;

			GetDlgItemText(hwndDlg, IDC_TIME, strTemp, sizeof(strTemp)/sizeof(_TCHAR));
			settings.nTimeout  = _ttoi(strTemp);
			settings.nTimeout += nm->iDelta;
			if (settings.nTimeout > 30 && nm->iDelta > 0)
				settings.nTimeout = 30;
			else if (settings.nTimeout > 30 && nm->iDelta < 0)
				settings.nTimeout = 0;
			_itot_s(settings.nTimeout, strTemp, sizeof(strTemp)/sizeof(_TCHAR), 10);
			SetDlgItemText(hwndDlg, IDC_TIME, strTemp);

			bReturn = TRUE;
		}

		break;
	}
	} // switch (uMsg)

	return bReturn;
}