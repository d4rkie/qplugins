#pragma once

#include "stdafx.h"

#include "QMPCLIEnc.h"


//////////////////////////////////////////////////////////////////////////

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUT };

	BEGIN_MSG_MAP_EX(CAboutDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		//MESSAGE_HANDLER(WM_CLOSE, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP_EX()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow( GetParent());

		m_ctlURLVendor.SubclassWindow( GetDlgItem( IDC_URL_VENDOR));
		m_ctlURLVendor.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLVendor.SetHyperLink( _T("http://sourceofrge.net/projects/qplugins/"));

		return TRUE;
	}

	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog( nID);
	}

private:
	CHyperLink m_ctlURLVendor;
};

