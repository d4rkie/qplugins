#include <TCHAR.h>
#include <windows.h>

#include "Audioscrobbler.h"
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