#pragma once

#include "stdafx.h"

#include "QMPCLIEnc.h"

#include "SettingsDlg.h"
#include "PresetsDlg.h"


//////////////////////////////////////////////////////////////////////////

class CParentDlg :
	public CDialogImpl< CParentDlg >, 
	public CWinDataExchange< CParentDlg >
{
public:
	CParentDlg(void) {}
	~CParentDlg(void) {}


public:
	enum { IDD = IDD_PARENT };

	// Maps
	BEGIN_MSG_MAP_EX(CParentDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		NOTIFY_HANDLER_EX(IDC_TAB, TCN_SELCHANGE, OnTcnSelChange)
		MESSAGE_HANDLER_EX(WM_PN_DIALOGSAVE, OnDialogSave)
	END_MSG_MAP_EX()

	// DDX
	BEGIN_DDX_MAP(CParentDlg)
		DDX_CONTROL_HANDLE(IDC_TAB, m_ctrlTab)
		DDX_CONTROL(IDC_URL_ENCODERS, m_ctrlURLEncoders)
	END_DDX_MAP()

	// Message handlers
	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CRect rect;

		// DDX controls, Hook it.
		DoDataExchange( FALSE);

		// Update the UI
		m_ctrlURLEncoders.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctrlURLEncoders.SetHyperLink( _T("http://www.rarewares.org/"));

		m_dlgSettings.Create( m_hWnd);
		m_ctrlTab.InsertItem( 0, TCIF_PARAM | TCIF_TEXT, _T("Settings"), -1, (DWORD)m_dlgSettings.m_hWnd);
		m_ctrlTab.GetItemRect( 0, &rect);
		m_dlgSettings.SetWindowPos( NULL, 0, rect.bottom, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_SHOWWINDOW);

		m_dlgPresets.Create( m_hWnd, (LPARAM)m_dlgSettings.m_hWnd); // pass the settings dialog to preset dialog for switching
		m_ctrlTab.InsertItem( 1, TCIF_PARAM | TCIF_TEXT, _T("Presets"), -1, (DWORD)m_dlgPresets.m_hWnd);
		m_ctrlTab.GetItemRect( 1, &rect);
		m_dlgPresets.SetWindowPos( NULL, 0, rect.bottom, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_HIDEWINDOW);

        m_ctrlTab.SetCurSel( 0);
		SetWindowLongPtr( GWLP_USERDATA, 0);

		return TRUE;
	}

	LRESULT OnTcnSelChange(LPNMHDR pNMHDR)
	{
		int pre, sel;
		TCITEM tie;

		ZeroMemory( &tie, sizeof ( TCITEM));
		tie.mask = TCIF_PARAM;

		pre = (int)GetWindowLongPtr( GWLP_USERDATA);
		sel = m_ctrlTab.GetCurSel();

		if ( pre >= 0 && sel >= 0) {
			m_ctrlTab.GetItem( pre, &tie);
			::ShowWindow( (HWND)tie.lParam, SW_HIDE);
			m_ctrlTab.GetItem( sel, &tie);
			::ShowWindow( (HWND)tie.lParam, SW_SHOW);

			SetWindowLongPtr( GWLP_USERDATA, sel);
		}

		return TRUE;
	}

	LRESULT OnDialogSave(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// notify all child windows
		if ( m_dlgSettings.IsWindow())
			m_dlgSettings.SendMessage( WM_PN_DIALOGSAVE);
		if ( m_dlgPresets.IsWindow())
			m_dlgPresets.SendMessage( WM_PN_DIALOGSAVE);

		return TRUE;
	}


private: // DDX Vars
	CTabCtrl m_ctrlTab;
	CHyperLink m_ctrlURLEncoders;


private: // Child dialog box
	CSettingsDlg m_dlgSettings;
	CPresetsDlg m_dlgPresets;
};

