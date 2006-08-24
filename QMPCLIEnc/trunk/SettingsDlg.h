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
	BEGIN_MSG_MAP(CSettingsDlg)
		CHAIN_MSG_MAP(CToolTipDialog< CSettingsDlg >)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BROWSE_PATH, OnBrowsePath)
		MESSAGE_HANDLER_EX(WM_PN_DIALOGSAVE, OnDialogSave)
		//MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
		//NOTIFY_CODE_HANDLER_EX(TTN_GETDISPINFO, OnToolTipNotify)
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

		//// Create the tool tip control
		//m_ctrlToolTip.Create( m_hWnd, NULL, NULL, TTS_NOPREFIX | TTS_BALLOON);
		//m_ctrlToolTip.SetMaxTipWidth( 500);

		//// add tools
		//CToolInfo ti(TTF_SUBCLASS, GetDlgItem( IDC_PARAMETER));
		//m_ctrlToolTip.AddTool( ti);

		// set property of tool tip control
		GetTT().SetDelayTime( TTDT_AUTOPOP, 10000); // set the show delay to 10 seconds
		GetTT().SetMaxTipWidth( 500); // enable multi-line tooltips

		return TRUE;
	}

	void OnBrowsePath(UINT uCode, int nID, HWND hwndCtrl)
	{
		CFileDialog fileDlg( TRUE, _T("exe"), NULL, OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST, _T("Executables (*.exe;*.vbs)\0*.exe;*.vbs\0All (*.*)\0*.*\0"), m_hWnd);

		if ( fileDlg.DoModal() == IDOK)
			SetDlgItemText( IDC_PATH, fileDlg.m_szFileName);
	}

	//LRESULT OnToolTipNotify(LPNMHDR lpNMHDR)
	//{
	//	LPNMTTDISPINFO lpTTDI = (LPNMTTDISPINFO)lpNMHDR;

	//	if ( lpTTDI->hdr.idFrom == (UINT_PTR)GetDlgItem( IDC_PARAMETER).m_hWnd) { // Tool tip for parameter edit control
	//		m_strToolTip.LoadString( IDS_TT_PARAMETER);
	//		lpTTDI->lpszText = (LPTSTR)(LPCTSTR)m_strToolTip;
	//	}

	//	return TRUE;
	//}

	//LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	//{
	//	MSG msg = { m_hWnd, uMsg, wParam, lParam };
	//	if ( m_ctrlToolTip.IsWindow())
	//		m_ctrlToolTip.RelayEvent( &msg);
	//	
	//	SetMsgHandled( FALSE);
	//	return TRUE;
	//}

	LRESULT OnDialogSave(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DoDataExchange( TRUE);
		return TRUE;
	}


private: // DDX Vars
	CToolTipCtrl m_ctrlToolTip;
	CString m_strToolTip;
};

