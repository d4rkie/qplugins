// About.cpp: implementation of the CAbout class.
//
//////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include "Help.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHelp::CHelp(HINSTANCE hAppInstance, HWND hAppHwnd)
{
	DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_HELP), hAppHwnd, DialogProc);
}

CHelp::~CHelp()
{

}

//////////////////////////////////////////////////////////////////////

BOOL CALLBACK CHelp::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		// ConvertStaticToHyperlink(hwndDlg, IDC_ABOUT_SFLINK);
		break;
	}
	case WM_COMMAND :
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hwndDlg, IDOK);
			return TRUE;
		}

	} // switch (uMsg)

	return FALSE;
}
