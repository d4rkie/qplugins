#pragma once

#include "stdafx.h"

#include "QMPCLIEnc.h"
#include "ToolTipDialog.h"


//////////////////////////////////////////////////////////////////////////

class CSettingsDlg :
	public CDialogImpl< CSettingsDlg >, 
	public CWinDataExchange< CSettingsDlg >, 
	public CToolTipDialog< CSettingsDlg >
{
public:
	CSettingsDlg(void) {}
	~CSettingsDlg(void) {}


public:
	enum { IDD = IDD_SETTINGS };

	// Maps
	BEGIN_MSG_MAP_EX(CSettingsDlg)
		CHAIN_MSG_MAP(CToolTipDialog< CSettingsDlg >)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BROWSE_PATH, OnBrowsePath)
		MESSAGE_HANDLER_EX(WM_PN_DIALOGSAVE, OnDialogSave)
	END_MSG_MAP_EX()

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
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		// DDX controls, Hook it.
		DoDataExchange( FALSE);

		// set property of tool tip control
		GetTT().SetDelayTime( TTDT_AUTOPOP, 10000); // set the show delay to 10 seconds
		GetTT().SetMaxTipWidth( 500); // enable multi-line tooltips

		// set icon and title of tool tip control
		TTSetTitle( IDC_PARAMETER, 1, _T("REQUIRED"));
		TTSetTitle( IDC_DO_TAG, 1, _T("RECOMMENDED"));
		TTSetTitle( IDC_NO_WAV_HEADER, 1, _T("NOT RECOMMENDED"));
		TTSetTitle( IDC_SHOW_CONSOLE, 1, _T("OPTIONAL"));

		return TRUE;
	}

	void OnBrowsePath(UINT uCode, int nID, HWND hwndCtrl)
	{
		CFileDialog fileDlg( TRUE, _T("exe"), NULL, OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST, _T("Executables (*.exe;*.vbs;*.js)\0*.exe;*.vbs;*.js\0All (*.*)\0*.*\0"), m_hWnd);

		if ( fileDlg.DoModal() == IDOK)
			SetDlgItemText( IDC_PATH, fileDlg.m_szFileName);
	}

	LRESULT OnDialogSave(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DoDataExchange( TRUE);
		return TRUE;
	}


private: // DDX Vars
	CToolTipCtrl m_ctrlToolTip;
	CString m_strToolTip;
};

