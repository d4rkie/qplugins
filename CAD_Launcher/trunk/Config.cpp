// Config.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <Commdlg.h>
#include <TCHAR.h>
#include "..\..\Includes\Win32Error.h"
#include "CADLauncher.h"
#include "Config.h"

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

BOOL CALLBACK CConfig::DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bReturn = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		UpdateCadPath(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_START_WITH_QMP, settings.bStartCad);
		CheckDlgButton(hwndDlg, IDC_CLOSE_WITH_QMP, settings.bCloseCad);		

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
			WCHAR strPath[MAX_PATH];

			settings.bStartCad = IsDlgButtonChecked(hwndDlg, IDC_START_WITH_QMP);
			settings.bCloseCad = IsDlgButtonChecked(hwndDlg, IDC_CLOSE_WITH_QMP);

			GetDlgItemTextW(hwndDlg, IDC_CAD_PATH, strPath, MAX_PATH);
			ParseCadPath((LPCWSTR)strPath);

			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		case IDCANCEL :
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		case IDC_BROWSE :
			BrowseForFile();
			UpdateCadPath(hwndDlg);
			break;

		case IDC_FIND :
			GetCadFilePath();
			UpdateCadPath(hwndDlg);
			break;

		} // switch (wParam)

		break;
	}
	} // switch (uMsg)

	return bReturn;
}

void CConfig::BrowseForFile()
{
	OPENFILENAME ofn;
	WCHAR strPath[_MAX_PATH];

	ZeroMemory(strPath, _MAX_PATH);
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize		= sizeof(ofn);
	ofn.hwndOwner		= hwndPlayer;
	ofn.lpstrFilter		= L"CAD.exe\0CAD.exe\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrTitle		= L"Specify location of CAD.exe";
	ofn.Flags			= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrFile		= strPath;
	ofn.nMaxFile		= _MAX_PATH;
	ofn.lpstrInitialDir = settings.strCadPath;

	if (GetOpenFileNameW(&ofn))
	{
		UINT nStrLen = wcslen(ofn.lpstrFile);
		UINT nFileSize = nStrLen - ofn.nFileOffset + 1;
		UINT nPathSize = ofn.nFileOffset + 1;

		FreeCadPath();
		settings.strCadFile = new _TCHAR[nFileSize];
		settings.strCadPath = new _TCHAR[nPathSize];	
		ZeroMemory(settings.strCadFile, nFileSize);
		ZeroMemory(settings.strCadPath, nPathSize);

		wcsncpy_s(settings.strCadPath, nPathSize, strPath, ofn.nFileOffset);
		wcscpy_s(settings.strCadFile, nFileSize, strPath + ofn.nFileOffset);
	}
	else
	{
		DWORD iExtErr = CommDlgExtendedError();
		if (iExtErr > 0) {	// Not sure this will work
			CWin32Error e;
			MessageBox(hwndPlayer, e, _T("Error"), MB_OK);
		}
	}
}

void CConfig::UpdateCadPath(HWND hwndDlg)
{
	WCHAR strCadPath[MAX_PATH];
	wcscpy_s(strCadPath, MAX_PATH, settings.strCadPath);
	wcscat_s(strCadPath, MAX_PATH, settings.strCadFile);

	SetDlgItemTextW(hwndDlg, IDC_CAD_PATH, strCadPath);
}