// Config.h: interface for the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIG_H__EDBF8BA7_F3BF_45AB_B7FC_807092EEDD95__INCLUDED_)
#define AFX_CONFIG_H__EDBF8BA7_F3BF_45AB_B7FC_807092EEDD95__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WinLIRC.h"

class CConfig  
{
public:
	CConfig();
	virtual ~CConfig();

	INT GetPageID();
	static VOID DialogInit(HWND hwndDlg);
	static VOID SetSettings(Settings* pSettings);

private:
	int m_iPageID;
	static Settings* m_pSettings;
	static HWND m_hwndConnStatus;
	static HWND m_hwndWinLIRCStatus;
	static HWND m_hwndReconnectBtn;
	static HWND m_hwndLaunchWinLIRCBtn;
	static HWND m_hwndList;
	static HWND m_hwndDlg;

	static BOOL CALLBACK CConfig::DialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void OnAddBtn();
	static void OnRemoveBtn();
	static void OnModifyBtn();
	static void OnReconnectBtn();
	static void OnListDblClk();
	static void OnOptionsBtn();
	static void OnHelpBtn();
	static void OnLaunchWinLIRCBtn();

	const static _TCHAR* Status_Connected;
	const static _TCHAR* Status_NotConnected;
	const static _TCHAR* Status_Running;
	const static _TCHAR* Status_NotRunning;

};

#endif // !defined(AFX_CONFIG_H__EDBF8BA7_F3BF_45AB_B7FC_807092EEDD95__INCLUDED_)
