#pragma once

#include "stdafx.h"
#include "QMPWV.h"


//////////////////////////////////////////////////////////////////////////

class CConfigDlg :
	public CDialogImpl< CConfigDlg >, 
	public CWinDataExchange< CConfigDlg >
{
public:
	CConfigDlg(void) {}
	~CConfigDlg(void) {}


public:
	enum { IDD = IDD_CONFIG };

	// Maps
	BEGIN_MSG_MAP_EX(CConfigDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER_EX(IDC_CLOSE, OnButtons)
		COMMAND_ID_HANDLER_EX(IDC_USEWVC, OnButtons)
	END_MSG_MAP_EX()

	// DDX
	BEGIN_DDX_MAP(CConfigDlg)
		DDX_CHECK(IDC_USEWVC, g_bUseWVC)
	END_DDX_MAP()

	// Message handlers
	LRESULT OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow();

		// Save the pointer to myself for self-destroy
		m_pME = (CConfigDlg **)lInitParam;

		// DDX controls, Hook it.
		DoDataExchange( FALSE);

		return TRUE;
	}

	void OnButtons(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		switch (nID)
		{
		case IDC_USEWVC:
			DoDataExchange( TRUE);

			break;
		case IDC_CLOSE:
			OnClose();

			break;
		}
	}

	void OnClose()
	{
		DestroyWindow();
	}
	void OnFinalMessage(HWND hwnd)
	{
		if ( m_pME) { // destroy myself automatically^_^
			*m_pME = NULL;
			delete this;
		}
	}


private: // DDX Vars
	CConfigDlg **m_pME;
};

