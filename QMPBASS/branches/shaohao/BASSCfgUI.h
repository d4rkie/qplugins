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

HWND DoConfigSheet(HINSTANCE hInstance, HWND hwndParent);

int CALLBACK ConfigSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
INT_PTR CALLBACK GeneralDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StreamingDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK StreamSavingDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND DoAboutDlg(HINSTANCE hInstance, HWND hwndParent);
INT_PTR CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND DoStreamSavingBar(HINSTANCE hInstance, HWND hwndParent);
INT_PTR CALLBACK StreamSavingBarProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

