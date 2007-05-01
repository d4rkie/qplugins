#if !defined(AFX_CONFIG_H__70C98A0B_AFC7_4680_9D01_EC9EB2C8EC12__INCLUDED_)
#define AFX_CONFIG_H__70C98A0B_AFC7_4680_9D01_EC9EB2C8EC12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
//#include <QCDModDefs.h>

//////////////////////////////////////////////////////////////////////

void LoadSettings();
void SaveSettings();

void CreateConfigDlg(HINSTANCE hInstance, HWND hAppHwnd);
BOOL CALLBACK ConfigDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_CONFIG_H__70C98A0B_AFC7_4680_9D01_EC9EB2C8EC12__INCLUDED_)
