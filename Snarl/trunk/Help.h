// About.h: interface for the CAbout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ABOUT_H__A594AAD1_4311_4D0E_8CEC_DF9106503717__INCLUDED_)
#define AFX_ABOUT_H__A594AAD1_4311_4D0E_8CEC_DF9106503717__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class CHelp  
{
public:
	CHelp(HINSTANCE hAppInstance, HWND hAppHwnd);
	virtual ~CHelp();

	static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // !defined(AFX_ABOUT_H__A594AAD1_4311_4D0E_8CEC_DF9106503717__INCLUDED_)
