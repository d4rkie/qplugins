#pragma once

#include "stdafx.h"
#include "QMPTAK.h"

#include "tak_deco_lib.h"


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

		m_ctlURLFLAC.SubclassWindow( GetDlgItem( IDC_LIB_URL));
		m_ctlURLFLAC.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLFLAC.SetHyperLink( _T("http://www.thbeck.de/Tak/Tak.html"));

		CStringW verStr;
		TtakInt32 Version, Compatibility;
		tak_GetLibraryVersion( &Version, &Compatibility);
		verStr.Format( _T("v%d.%d.%d"), (Version & 0xFF0000) >> 16, (Version & 0xFF00) >> 8, (Version & 0xFF));

		SetDlgItemText( IDC_LIB_VER, verStr);

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

