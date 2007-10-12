#pragma once

#include "QMPCUE.h"


//////////////////////////////////////////////////////////////////////////

class CGenerateCUESheetDlg : public CDialogImpl< CGenerateCUESheetDlg >
{
public:
	enum { IDD = IDD_GENERATE_CUE_SHEET };

	BEGIN_MSG_MAP_EX(CGenerateCUESheetDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		//MESSAGE_HANDLER(WM_CLOSE, OnCloseCmd)
		//COMMAND_ID_HANDLER_EX(IDOK, OnCloseCmd)
		//COMMAND_ID_HANDLER_EX(IDCANCEL, OnCloseCmd)
	END_MSG_MAP_EX()

	BEGIN_DDX_MAP(CGenerateCUESheetDlg)
		DDX_CONTROL_HANDLE(IDC_LIST_TRACKS, m_ctrlTrackList)
	END_DDX_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		CenterWindow();

		m_ctrlTrackList.AddColumn( _T("No."), 0);
		m_ctrlTrackList.AddColumn( _T("Start"), 1);
		m_ctrlTrackList.AddColumn( _T("End"), 2);
		m_ctrlTrackList.AddColumn( _T("File"), 3);

		return TRUE;
	}

private:
	CListViewCtrl m_ctrlTrackList;
};

