// Config.h: interface for the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CCONFIG_H)
#define CCONFIG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class CConfig  
{
public:
	CConfig();
	CConfig(HINSTANCE hInstance, HWND hAppHwnd);
	virtual ~CConfig();

private:
	static BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void BrowseForFile();
	static void UpdateCadPath(HWND hwndDlg);
};

#endif // CCONFIG_H