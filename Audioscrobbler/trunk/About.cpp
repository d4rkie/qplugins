#include "Precompiled.h"
#include "AudioscrobblerDLL.h"
#include "About.h"

//////////////////////////////////////////////////////////////////////

void CreateAboutDlg(HINSTANCE hInstance, HWND hAppHwnd)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hAppHwnd, AboutDlgProc);
}

//////////////////////////////////////////////////////////////////////

BOOL CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bReturn = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		WCHAR* pCustomText =
			L"Please report bugs to either my e-mail or post on the Quinnware.com forum.\n\n"
			L"A thank you to the Audioscrobbler team for their great service!\n" 
			L"Also a thank you to the early testers and to Paul for QMP and a great plug-in development kit.";
		SendMessage(GetDlgItem(hwndDlg, IDC_ABOUT_CUSTOMTEXT), WM_SETTEXT, NULL, (LPARAM)pCustomText);

		HFONT hFont = (HFONT)GetStockObject(SYSTEM_FONT);
		SendMessage(GetDlgItem(hwndDlg, IDC_ABOUT_AUTHORS), WM_SETFONT, (WPARAM)hFont, TRUE);

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
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;
		} // switch (wParam)

		break;
	}
	}

	return bReturn;
}