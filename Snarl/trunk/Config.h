// Config.h
//
//////////////////////////////////////////////////////////////////////

#ifndef SNARL_CONFIG_H
#define SNARL_CONFIG_H

#define _WIN32_WINNT 0x0500
#define WINVER       0x0500


#include <windows.h>
#include <objbase.h>
#include <commctrl.h>
#include <shlobj.h>
#include <TCHAR.h>
#include <QString.h>
#include "QCDGeneralDLL.h"
#include "Help.h"
#include "resource.h"

#include "SnarlInterface.h"


//////////////////////////////////////////////////////////////////////

void BrowseForRootFolder();
void CheckRootFolder();

//////////////////////////////////////////////////////////////////////

const static TCHAR* strComboBox[4] = {_T("Title"), _T("Artist"), _T("Album"), _T("Nothing")};
const static PluginServiceOp ServiceOps[4] = {opGetTrackName, opGetArtistName, opGetDiscName, (PluginServiceOp)0};

//////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bReturn = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		LONG32 nBuild;
		WORD nHi, nLo;
		TCHAR str[64];
		HWND hwndCombos[3];

		// Fill combobox with standard stuff
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
		SetDlgItemInt(hwndDlg, IDC_TIME, Settings.nTimeout, FALSE);
		
		for (int i = 0; i < 4; i++)
		{
			if (Settings.Headline_ServiceOp == ServiceOps[i])
				SendMessage(hwndCombos[0], CB_SETCURSEL, i, 0);
			if (Settings.Text1_ServiceOp == ServiceOps[i])
				SendMessage(hwndCombos[1], CB_SETCURSEL, i, 0);
			if (Settings.Text2_ServiceOp == ServiceOps[i])
				SendMessage(hwndCombos[2], CB_SETCURSEL, i, 0);
		}
		
		CheckDlgButton(hwndDlg, IDC_CASCADE_POPUPS, Settings.bCascade);
		CheckDlgButton(hwndDlg, IDC_HEADLINE_WRAP,  Settings.bHeadline_wrap);
		CheckDlgButton(hwndDlg, IDC_START_SNARL,  Settings.bStartSnarl);
		CheckDlgButton(hwndDlg, IDC_CLOSE_SNARL,  Settings.bCloseSnarl);
		
		//CheckDlgButton(hwndDlg, IDC_TEXT1_WRAP,     Settings.bText1_wrap);
		//CheckDlgButton(hwndDlg, IDC_TEXT2_WRAP,     Settings.bText2_wrap);

		
		// Cover Art
		CheckDlgButton(hwndDlg, IDC_COVERART_DISPLAY,  Settings.bDisplayCoverArt);
		SetDlgItemText(hwndDlg, IDC_COVERART_ROOT,     Settings.strCoverArtRoot);
		SetDlgItemText(hwndDlg, IDC_COVERART_TEMPLATE, Settings.strCoverArtTemplate);


		// Snarl version
		if (snarl->snGetVersion(&nHi, &nLo))
			_stprintf_s(str, 64, _T("%u.%u"), nHi, nLo);
		else { // Try again, might have timeout
			if (snarl->snGetVersion(&nHi, &nLo))
				_stprintf_s(str, 64, _T("%u.%u"), nHi, nLo);
			else
				StringCbCopy(str, 64, _T("Failed to get Snarl version"));
		}
		SetDlgItemText(hwndDlg, IDC_SNARL_VERSION, str);

		// Snarl build
		nBuild = snarl->snGetVersionEx();
		if (nBuild != SnarlInterface::M_TIMED_OUT && nBuild != SnarlInterface::M_FAILED) {
			if (nBuild < 37)
				_stprintf_s(str, 64, _T("%u ***You need to upgrade Snarl***"), nBuild);
			else
				_stprintf_s(str, 64, _T("%u"), nBuild);
		}
		else
			_stprintf_s(str, 64, _T("Failed to get build #"));
		SetDlgItemText(hwndDlg, IDC_SNARL_BUILD, str);
		

		bReturn = TRUE;
		break;
	}
	case WM_PN_DIALOGSAVE : // Save work - QCD will destroy the dialog
	{
		TCHAR strBuff[32];
		TCHAR strTemp[MAX_PATH];
		HWND hwndCombos[3];

		hwndCombos[0] = GetDlgItem(hwndDlg, IDC_HEADLINE_COMBO);
		hwndCombos[1] = GetDlgItem(hwndDlg, IDC_TEXT1_COMBO);
		hwndCombos[2] = GetDlgItem(hwndDlg, IDC_TEXT2_COMBO);

		// Validate and save
		Settings.bCascade       = IsDlgButtonChecked(hwndDlg, IDC_CASCADE_POPUPS);
		Settings.bHeadline_wrap = IsDlgButtonChecked(hwndDlg, IDC_HEADLINE_WRAP);
		Settings.bStartSnarl    = IsDlgButtonChecked(hwndDlg, IDC_START_SNARL);
		Settings.bCloseSnarl    = IsDlgButtonChecked(hwndDlg, IDC_CLOSE_SNARL);

		//Settings.bText1_wrap    = IsDlgButtonChecked(hwndDlg, IDC_TEXT1_WRAP);
		//Settings.bText2_wrap    = IsDlgButtonChecked(hwndDlg, IDC_TEXT2_WRAP);
		
		Settings.Headline_ServiceOp = ServiceOps[SendMessage(hwndCombos[0], CB_GETCURSEL, 0, 0)];
		Settings.Text1_ServiceOp    = ServiceOps[SendMessage(hwndCombos[1], CB_GETCURSEL, 0, 0)];
		Settings.Text2_ServiceOp    = ServiceOps[SendMessage(hwndCombos[2], CB_GETCURSEL, 0, 0)];

		if (GetDlgItemText(hwndDlg, IDC_TIME, strBuff, sizeof(strBuff)/sizeof(_TCHAR)))
			Settings.nTimeout = _ttoi(strBuff);


		// Cover art
		Settings.bDisplayCoverArt = IsDlgButtonChecked(hwndDlg, IDC_COVERART_DISPLAY);
		
		GetDlgItemText(hwndDlg, IDC_COVERART_ROOT, strTemp, MAX_PATH);
		Settings.strCoverArtRoot = strTemp;
		CheckRootFolder();
		GetDlgItemText(hwndDlg, IDC_COVERART_TEMPLATE, strTemp, MAX_PATH);
		Settings.strCoverArtTemplate = strTemp;

		
		if (IsPlaying())
			DisplaySongInfo();

		bReturn = TRUE;
		break;
	}

	case WM_COMMAND :
	{
		switch (wParam)
		{
			case IDC_COVERART_HELP : {
				CHelp help(hInstance, hwndPlayer);
				break;
			}
			case IDC_COVERART_BROWSE :
				BrowseForRootFolder();
				SetDlgItemText(hwndDlg, IDC_COVERART_ROOT, Settings.strCoverArtRoot);
				break;
		}
		break;
	}
	case WM_NOTIFY :
	{
		if (wParam == IDC_TIME_SPIN) {
			_TCHAR strTemp[64];
			NMUPDOWN* nm = (NMUPDOWN*)lParam;

			GetDlgItemText(hwndDlg, IDC_TIME, strTemp, sizeof(strTemp)/sizeof(_TCHAR));
			Settings.nTimeout  = _ttoi(strTemp);
			Settings.nTimeout += nm->iDelta;
			if (Settings.nTimeout > 30 && nm->iDelta > 0)
				Settings.nTimeout = 30;
			else if (Settings.nTimeout > 30 && nm->iDelta < 0)
				Settings.nTimeout = 0;
			_itot_s(Settings.nTimeout, strTemp, sizeof(strTemp)/sizeof(_TCHAR), 10);
			SetDlgItemText(hwndDlg, IDC_TIME, strTemp);

			bReturn = TRUE;
		}

		break;
	}
	} // switch (uMsg)

	return bReturn;
}


void CheckRootFolder()
{
	if (Settings.strCoverArtRoot.substr(0, 12) == L"%CURRENT_DIR")
		return;
	if (Settings.strCoverArtRoot.at(Settings.strCoverArtRoot.length() - 1))
		Settings.strCoverArtRoot.push_back('\\');
}


void BrowseForRootFolder()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	TCHAR strDispName[MAX_PATH];

	BROWSEINFO bi;
	bi.hwndOwner      = hwndPlayer;
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = strDispName;
	bi.lpszTitle      = _T("Select template root folder");
	bi.ulFlags        = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_RETURNONLYFSDIRS;
	bi.lpfn           = NULL;
	bi.lParam         = 0;
	bi.iImage         = NULL;
	
	ITEMIDLIST* pidl = SHBrowseForFolder(&bi);

	if ( pidl != 0 )
	{
		TCHAR szPath[MAX_PATH];
		if (SHGetPathFromIDList(pidl, szPath)) {
			Settings.strCoverArtRoot.assign(szPath);
			Settings.strCoverArtRoot.push_back('\\');
		}

        // free memory used
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }
	}

	CoUninitialize();
}

#endif // SNARL_CONFIG_H