#pragma once

#include "QMPCUE.h"


//////////////////////////////////////////////////////////////////////////

class CConfigDlgTags :
	public CDialogImpl< CConfigDlgTags >, 
	public CWinDataExchange< CConfigDlgTags >
{
public:
	CConfigDlgTags(void) {}
	~CConfigDlgTags(void) {}


public:
	enum { IDD = IDD_CONFIG_TAGS };

	// Maps
	BEGIN_MSG_MAP_EX(CConfigDlgTags)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER_EX(IDOK, OnOKCmd)
	END_MSG_MAP_EX()

	// DDX
	BEGIN_DDX_MAP(CConfigDlgTags)
		DDX_CHECK(IDC_CHECK_OW_IMG_TAGS, g_bOWIMGTags)
	END_DDX_MAP()

	// Message handlers
	LRESULT OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow();

		// DDX controls, Hook it.
		DoDataExchange( FALSE);

		return TRUE;
	}

	void OnClose()
	{
		EndDialog(0);
	}
	LRESULT OnOKCmd(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		DoDataExchange(TRUE);

		EndDialog(nID);
		return 0;
	}
};

