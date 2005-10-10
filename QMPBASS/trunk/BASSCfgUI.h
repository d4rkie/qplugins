#pragma once

#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <prsht.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

// some defination for the latest PSDK
#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE	0x0040
#define BIF_USENEWUI (BIF_NEWDIALOGSTYLE | BIF_EDITBOX)
#endif

#ifndef PSM_HWNDTOINDEX
#define PSM_HWNDTOINDEX            (WM_USER + 129)
#define PropSheet_HwndToIndex(hDlg, hwnd) \
        (int)SNDMSG(hDlg, PSM_HWNDTOINDEX, (WPARAM)(hwnd), 0)
#endif
#ifndef PSM_INDEXTOHWND
#define PSM_INDEXTOHWND            (WM_USER + 130)
#define PropSheet_IndexToHwnd(hDlg, i) \
        (HWND)SNDMSG(hDlg, PSM_INDEXTOHWND, (WPARAM)(i), 0)
#endif

// UI Functions
HWND DoConfigSheet(HINSTANCE hInstance, HWND hwndParent);

int CALLBACK ConfigSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
INT_PTR CALLBACK GeneralDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StreamingDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StreamSavingDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddonsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND DoAboutDlg(HINSTANCE hInstance, HWND hwndParent);
INT_PTR CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND DoStreamSavingBar(HINSTANCE hInstance, HWND hwndParent);
INT_PTR CALLBACK StreamSavingBarProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

