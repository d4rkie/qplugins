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
	BEGIN_MSG_MAP(CConfigDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER_EX(IDC_CLOSE, OnClose)
		COMMAND_ID_HANDLER_EX(IDC_USEWVC, OnUseWVC)
	END_MSG_MAP()

	// DDX
	BEGIN_DDX_MAP(CConfigDlg)
		DDX_CHECK(IDC_USEWVC, g_bUseWVC)
	END_DDX_MAP()

	// Message handlers
	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		CenterWindow();

		// Save the pointer to myself for self-destroy
		m_pME = (CConfigDlg **)lParam;

		// DDX controls, Hook it.
		DoDataExchange( FALSE);

		return TRUE;
	}

	void OnUseWVC(UINT uCode, int nID, HWND hwndCtrl)
	{
		DoDataExchange( TRUE);
	}

	void OnClose(UINT uCode = 0, int nID = 0, HWND hwndCtrl = NULL)
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

