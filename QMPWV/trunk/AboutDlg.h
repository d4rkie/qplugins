#pragma once

#include "stdafx.h"
#include "QMPWV.h"

#include "wavpack.h"


//////////////////////////////////////////////////////////////////////////

class CAboutDlgInput : public CDialogImpl< CAboutDlgInput >
{
public:
	enum { IDD = IDD_ABOUT };

	BEGIN_MSG_MAP_EX(CAboutDlgInput)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP_EX()

	LRESULT OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow();

		m_ctlURLWavPack.SubclassWindow( GetDlgItem( IDC_LIB_URL));
		m_ctlURLWavPack.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLWavPack.SetHyperLink( _T("http://www.wavpack.com"));

		SetDlgItemTextA( m_hWnd, IDC_LIB_VER, WavpackGetLibraryVersionString());

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
	CHyperLink m_ctlURLWavPack;
};

