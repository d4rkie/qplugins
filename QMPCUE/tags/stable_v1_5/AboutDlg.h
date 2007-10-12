#pragma once

#include "QMPCUE.h"


//////////////////////////////////////////////////////////////////////////

class CAboutDlgInput : public CDialogImpl< CAboutDlgInput >
{
public:
	enum { IDD = IDD_ABOUT_INPUT };

	BEGIN_MSG_MAP_EX(CAboutDlgInput)
		MSG_WM_INITDIALOG(OnInitDialog)
		//MESSAGE_HANDLER(WM_CLOSE, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP_EX()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow();

		m_ctlURLQPlugins.SubclassWindow( GetDlgItem( IDC_URL_QPLUGINS));
		m_ctlURLQPlugins.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLQPlugins.SetHyperLink( _T("http://sourceofrge.net/projects/qplugins/"));

		return TRUE;
	}

	LRESULT OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog(nID);
		return 0;
	}

private:
	CHyperLink m_ctlURLQPlugins;
};


class CAboutDlgPlaylists : public CDialogImpl< CAboutDlgPlaylists >
{
public:
	enum { IDD = IDD_ABOUT_TAGS };

	BEGIN_MSG_MAP_EX(CAboutDlgPlaylists)
		MSG_WM_INITDIALOG(OnInitDialog)
		//MESSAGE_HANDLER(WM_CLOSE, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP_EX()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow();

		m_ctlURLQPlugins.SubclassWindow( GetDlgItem( IDC_URL_QPLUGINS));
		m_ctlURLQPlugins.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLQPlugins.SetHyperLink( _T("http://sourceofrge.net/projects/qplugins/"));

		return TRUE;
	}

	LRESULT OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog(nID);
		return 0;
	}

private:
	CHyperLink m_ctlURLQPlugins;
};


class CAboutDlgTags : public CDialogImpl< CAboutDlgTags >
{
public:
	enum { IDD = IDD_ABOUT_TAGS };

	BEGIN_MSG_MAP_EX(CAboutDlgTags)
		MSG_WM_INITDIALOG(OnInitDialog)
		//MESSAGE_HANDLER(WM_CLOSE, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP_EX()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow();

		m_ctlURLQPlugins.SubclassWindow( GetDlgItem( IDC_URL_QPLUGINS));
		m_ctlURLQPlugins.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLQPlugins.SetHyperLink( _T("http://sourceofrge.net/projects/qplugins/"));

		return TRUE;
	}

	LRESULT OnCloseCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		EndDialog(nID);
		return 0;
	}

private:
	CHyperLink m_ctlURLQPlugins;
};

