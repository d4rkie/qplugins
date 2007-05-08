#pragma once

#include "stdafx.h"
#include "QMPFLAC.h"


//////////////////////////////////////////////////////////////////////////

class CAboutDlgInput : public CDialogImpl< CAboutDlgInput >
{
public:
	enum { IDD = IDD_ABOUT_INPUT };

	BEGIN_MSG_MAP(CAboutDlgInput)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//MESSAGE_HANDLER(WM_CLOSE, OnCloseCmd)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow();

		m_ctlURLFLAC.SubclassWindow( GetDlgItem( IDC_URL_FLAC));
		m_ctlURLFLAC.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLFLAC.SetHyperLink( _T("http://flac.sourceforge.net"));

		SetDlgItemText( IDC_FLAC_VERNUM, _T("v1.1.4"));

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
	CHyperLink m_ctlURLFLAC;
};

class CAboutDlgTags : public CDialogImpl< CAboutDlgTags >
{
public:
	enum { IDD = IDD_ABOUT_TAGS };

	BEGIN_MSG_MAP(CAboutDlgTags)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//MESSAGE_HANDLER(WM_CLOSE, OnCloseCmd)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow();

		m_ctlURLFLAC.SubclassWindow( GetDlgItem( IDC_URL_FLAC));
		m_ctlURLFLAC.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLFLAC.SetHyperLink( _T("http://flac.sourceforge.net"));

		SetDlgItemText( IDC_FLAC_VERNUM, _T("v1.1.4"));

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
	CHyperLink m_ctlURLFLAC;
};

