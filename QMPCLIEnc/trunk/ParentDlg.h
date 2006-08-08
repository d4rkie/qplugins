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
	BEGIN_MSG_MAP(CParentDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		NOTIFY_HANDLER_EX(IDC_TAB, TCN_SELCHANGE, OnTcnSelChange)
		MESSAGE_HANDLER_EX(WM_PN_DIALOGSAVE, OnDialogSave)
	END_MSG_MAP()

	// DDX
	BEGIN_DDX_MAP(CParentDlg)
		DDX_CONTROL_HANDLE(IDC_TAB, m_ctrlTab)
		DDX_CONTROL(IDC_URL_ENCODERS, m_ctrlURLEncoders)
	END_DDX_MAP()

	// Message handlers
	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		CRect rect;

		// DDX controls, Hook it.
		DoDataExchange( FALSE);

		// Update the UI
		m_ctrlURLEncoders.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctrlURLEncoders.SetHyperLink( _T("http://www.rarewares.org/"));

        m_pdlgSettings = new CSettingsDlg;
		m_pdlgSettings->Create( m_hWnd);
		m_ctrlTab.InsertItem( 0, TCIF_PARAM | TCIF_TEXT, _T("Settings"), -1, (DWORD)m_pdlgSettings->m_hWnd);
		m_ctrlTab.GetItemRect( 0, &rect);
		m_pdlgSettings->SetWindowPos( NULL, 0, rect.bottom, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_SHOWWINDOW);

		m_pdlgPresets = new CPresetsDlg;
		m_pdlgPresets->Create( m_hWnd);
		m_ctrlTab.InsertItem( 1, TCIF_PARAM | TCIF_TEXT, _T("Presets"), -1, (DWORD)m_pdlgPresets->m_hWnd);
		m_ctrlTab.GetItemRect( 1, &rect);
		m_pdlgPresets->SetWindowPos( NULL, 0, rect.bottom, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_HIDEWINDOW);

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
			::SetFocus( (HWND)tie.lParam);

			SetWindowLongPtr( GWLP_USERDATA, sel);
		}

		return TRUE;
	}

	LRESULT OnDialogSave(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// destroy all child windows
		if ( m_pdlgSettings) {
			if ( m_pdlgSettings->IsWindow()) {
				m_pdlgSettings->SendMessage( WM_PN_DIALOGSAVE);
				m_pdlgSettings->DestroyWindow();
			}

			delete m_pdlgSettings; m_pdlgSettings = NULL;
		}
		if ( m_pdlgPresets) {
			if ( m_pdlgPresets->IsWindow()) {
				m_pdlgPresets->SendMessage( WM_PN_DIALOGSAVE);
				m_pdlgPresets->DestroyWindow();
			}

			delete m_pdlgPresets; m_pdlgPresets = NULL;
		}

		return TRUE;
	}


private: // DDX Vars
	CTabCtrl m_ctrlTab;
	CHyperLink m_ctrlURLEncoders;


private: // Child dialog box
	CSettingsDlg * m_pdlgSettings;
	CPresetsDlg * m_pdlgPresets;
};

