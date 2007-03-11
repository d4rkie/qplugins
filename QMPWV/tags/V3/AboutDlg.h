#pragma once

#include "stdafx.h"
#include "QMPWV.h"


//////////////////////////////////////////////////////////////////////////

class CAboutDlg : public CDialogImpl< CAboutDlg >
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
		CenterWindow();

		m_ctlURLWavPack.SubclassWindow( GetDlgItem( IDC_URL_WAVPACK));
		m_ctlURLWavPack.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLWavPack.SetHyperLink( _T("http://www.wavpack.com"));

		SetDlgItemText( IDC_WAVPACK_VERNUM, _T("v4.31"));

		m_ctlURLQPlugins.SubclassWindow( GetDlgItem( IDC_URL_QPLUGINS));
		m_ctlURLQPlugins.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLQPlugins.SetHyperLink( _T("http://sourceofrge.net/projects/qplugins/"));

		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

private:
	CHyperLink m_ctlURLQPlugins;
	CHyperLink m_ctlURLWavPack;
};

