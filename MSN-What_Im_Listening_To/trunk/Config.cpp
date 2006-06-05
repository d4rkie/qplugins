// Config.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>
#include <TCHAR.h>
#include "QCDGeneralDLL.h"
#include "Config.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig::CConfig(HINSTANCE hInstance, HWND hAppHwnd)
{
	if (nMSNBuild >= 566)
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG8), hAppHwnd, DlgProc8);
	else
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG7), hAppHwnd, DlgProc7);
}

CConfig::~CConfig()
{

}

//////////////////////////////////////////////////////////////////////

BOOL CALLBACK CConfig::DlgProc7(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bReturn = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		CheckDlgButton(hwndDlg, IDC_CONFIG_ARTIST,   settings.bArtist);
		CheckDlgButton(hwndDlg, IDC_CONFIG_ALBUM,    settings.bAlbum);
		CheckDlgButton(hwndDlg, IDC_CONFIG_SHOWVID,  settings.bVideo);

		SendDlgItemMessage(hwndDlg, IDC_CONFIG_DELAY_SPIN, UDM_SETRANGE, 0, MAKELONG(30, 0));
		
		if (RegDB_GetWMPVersion() >= 9)
			SetDlgItemText(hwndDlg, IDC_CONFIG_FIXWMP, L"Remove WMP faking");
		else
			SetDlgItemText(hwndDlg, IDC_CONFIG_FIXWMP, L"Fake WMP install");
		
		SetDlgItemInt(hwndDlg, IDC_CONFIG_DELAY, settings.nDelay / 1000, FALSE);

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

			// Validate and save
			settings.bArtist   = IsDlgButtonChecked(hwndDlg, IDC_CONFIG_ARTIST);
			settings.bAlbum    = IsDlgButtonChecked(hwndDlg, IDC_CONFIG_ALBUM);
			settings.bVideo    = IsDlgButtonChecked(hwndDlg, IDC_CONFIG_SHOWVID);

			if (GetDlgItemText(hwndDlg, IDC_CONFIG_DELAY, strBuff, sizeof(strBuff)/sizeof(_TCHAR)))
				settings.nDelay = _ttoi(strBuff) * 1000;

			if (IsPlaying())
				UpdateSong();

			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		case IDCANCEL :
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		case IDC_CONFIG_FIXWMP :
			if (RegDB_GetWMPVersion() < 9) {
				if (IDOK == MessageBox(hwndPlayer, L"Are you sure you want to add regdb entries\nfor Windows Media Player recognition?", L"QMP \"What Im Listening To\"", MB_OKCANCEL | MB_ICONQUESTION)) {
					RegDB_Fix(TRUE);
					SetDlgItemText(hwndDlg, IDC_CONFIG_FIXWMP, L"Remove WMP faking");
				}
			}
			else {
				if (IDOK == MessageBox(hwndPlayer, L"Are you sure you want to remove the regdb entries\nfor Windows Media Player recognition?", L"QMP \"What Im Listening To\"", MB_OKCANCEL | MB_ICONQUESTION)) {
					RegDB_Fix(FALSE);
					SetDlgItemText(hwndDlg, IDC_CONFIG_FIXWMP, L"Fake WMP install");
				}
			}
			break;
		} // switch (wParam)

		break;
	}
	case WM_NOTIFY :
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
	}
	} // switch (uMsg)

	return bReturn;
}

//////////////////////////////////////////////////////////////////////
// Dialog prog for build 566+
//////////////////////////////////////////////////////////////////////
BOOL CALLBACK CConfig::DlgProc8(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bReturn = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		CheckDlgButton(hwndDlg, IDC_CONFIG_SHOWVID,  settings.bVideo);
		SendDlgItemMessage(hwndDlg, IDC_CONFIG_DELAY_SPIN, UDM_SETRANGE, 0, MAKELONG(30, 0));
		
		if (RegDB_GetWMPVersion() >= 9)
			SetDlgItemText(hwndDlg, IDC_CONFIG_FIXWMP, L"Remove WMP faking");
		else
			SetDlgItemText(hwndDlg, IDC_CONFIG_FIXWMP, L"Fake WMP install");
		
		SetDlgItemInt(hwndDlg, IDC_CONFIG_DELAY, settings.nDelay / 1000, FALSE);

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

			// Validate and save
			settings.bVideo    = IsDlgButtonChecked(hwndDlg, IDC_CONFIG_SHOWVID);

			if (GetDlgItemText(hwndDlg, IDC_CONFIG_DELAY, strBuff, sizeof(strBuff)/sizeof(_TCHAR)))
				settings.nDelay = _ttoi(strBuff) * 1000;

			if (IsPlaying())
				UpdateSong();

			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		case IDCANCEL :
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		case IDC_CONFIG_FIXWMP :
			if (RegDB_GetWMPVersion() < 9) {
				if (IDOK == MessageBox(hwndPlayer, L"Are you sure you want to add regdb entries\nfor Windows Media Player recognition?", L"QMP \"What Im Listening To\"", MB_OKCANCEL | MB_ICONQUESTION)) {
					RegDB_Fix(TRUE);
					SetDlgItemText(hwndDlg, IDC_CONFIG_FIXWMP, L"Remove WMP faking");
				}
			}
			else {
				if (IDOK == MessageBox(hwndPlayer, L"Are you sure you want to remove the regdb entries\nfor Windows Media Player recognition?", L"QMP \"What Im Listening To\"", MB_OKCANCEL | MB_ICONQUESTION)) {
					RegDB_Fix(FALSE);
					SetDlgItemText(hwndDlg, IDC_CONFIG_FIXWMP, L"Fake WMP install");
				}
			}
			break;
		} // switch (wParam)

		break;
	}
	case WM_NOTIFY :
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
	}
	} // switch (uMsg)

	return bReturn;
}