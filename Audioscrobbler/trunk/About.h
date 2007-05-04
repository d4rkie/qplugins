#if !defined(AUDIOSCROBBLER_ABOUT_H)
#define AUDIOSCROBBLER_ABOUT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

//////////////////////////////////////////////////////////////////////

void CreateAboutDlg(HINSTANCE hInstance, HWND hAppHwnd);
BOOL CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//////////////////////////////////////////////////////////////////////

#endif // !defined(AUDIOSCROBBLER_ABOUT_H)
