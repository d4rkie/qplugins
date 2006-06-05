// Config.h: interface for the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIG_H__70C98A0B_AFC7_4680_9D01_EC9EB2C8EC12__INCLUDED_)
#define AFX_CONFIG_H__70C98A0B_AFC7_4680_9D01_EC9EB2C8EC12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CConfig  
{
public:
	CConfig(HINSTANCE hInstance, HWND hAppHwnd);
	virtual ~CConfig();

private:
	static BOOL CALLBACK DlgProc7(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DlgProc8(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // !defined(AFX_CONFIG_H__70C98A0B_AFC7_4680_9D01_EC9EB2C8EC12__INCLUDED_)
