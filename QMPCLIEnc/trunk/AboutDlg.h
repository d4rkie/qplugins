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
		COMMAND_ID_HANDLER_EX(IDC_ACKNOWLEDGE, OnAcknowledge)
	END_MSG_MAP_EX()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow( GetParent());

		m_ctlURLVendor.SubclassWindow( GetDlgItem( IDC_URL_VENDOR));
		m_ctlURLVendor.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLVendor.SetHyperLink( _T("http://sourceofrge.net/projects/qplugins/"));

		return TRUE;
	}

	void OnAcknowledge(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		LPCTSTR acklist = _T("These people make this plug-in more perfect!\n\nThess");
		MessageBox( acklist, _T("Acknowledge"));
	}

	void OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog( nID);
	}

private:
	CHyperLink m_ctlURLVendor;
};

