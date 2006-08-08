#pragma once

#include "stdafx.h"

#include "QMPCLIEnc.h"


//////////////////////////////////////////////////////////////////////////

class CSettingsDlg :
	public CDialogImpl< CSettingsDlg >, 
	public CWinDataExchange< CSettingsDlg >
{
public:
	CSettingsDlg(void) {}
	~CSettingsDlg(void) {}


public:
	enum { IDD = IDD_SETTINGS };

	// Maps
	BEGIN_MSG_MAP(CSettingsDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BROWSE_PATH, OnBrowsePath)
		MESSAGE_HANDLER_EX(WM_PN_DIALOGSAVE, OnDialogSave)
	END_MSG_MAP()

	// DDX
	BEGIN_DDX_MAP(CSettingsDlg)
		DDX_TEXT(IDC_PATH, g_strPath)
		DDX_TEXT(IDC_PARAMETER, g_strParameter)
		DDX_TEXT(IDC_EXTENSION, g_strExtension)
		DDX_CHECK(IDC_DO_TAG, g_bDoTag)
		DDX_CHECK(IDC_NO_WAV_HEADER, g_bNoWAVHeader)
		DDX_CHECK(IDC_SHOW_CONSOLE, g_bShowConsole)
	END_DDX_MAP()

	// Message handlers
	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		// DDX controls, Hook it.
		DoDataExchange( FALSE);

		return TRUE;
	}

	void OnBrowsePath(UINT uCode, int nID, HWND hwndCtrl)
	{
		CFileDialog fileDlg( TRUE, _T("exe"), NULL, OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST, _T("Executables (*.exe;*.vbs)\0*.exe;*.vbs\0All (*.*)\0*.*\0"), m_hWnd);

		if ( fileDlg.DoModal() == IDOK)
			SetDlgItemText( IDC_PATH, fileDlg.m_szFileName);
	}

	LRESULT OnDialogSave(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DoDataExchange( TRUE);
		return TRUE;
	}


private: // DDX Vars
};

