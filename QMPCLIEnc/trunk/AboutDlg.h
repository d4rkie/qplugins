#pragma once

#include "stdafx.h"

#include "QMPCLIEnc.h"


//////////////////////////////////////////////////////////////////////////

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUT };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//MESSAGE_HANDLER(WM_CLOSE, OnCloseCmd)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow( GetParent());

		SetDlgItemText( IDC_PLUGIN_VERSION, PLUGIN_VERSION);

		m_ctlURLVendor.SubclassWindow( GetDlgItem( IDC_URL_VENDOR));
		m_ctlURLVendor.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLVendor.SetHyperLink( _T("http://sourceofrge.net/projects/qplugins/"));

		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

private:
	CHyperLink m_ctlURLVendor;
};

