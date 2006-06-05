//-----------------------------------------------------------------------------
// 
// File:	QCDInputDLL.h
//
// About:	QCD Player Input module DLL interface.  For more documentation, see
//			QCDModInput.h.
//
// Authors:	Written by Paul Quinn and Richard Carlson.
//
//	QCD multimedia player application Software Development Kit Release 1.0.
//
//	Copyright (C) 1997-2002 Quinnware
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

#ifndef QCDGeneralDLL_H
#define QCDGeneralDLL_H

#include "..\qcdpdk\QCDGeneral\QCDModGeneral.h"
#include "..\qcdpdk\QCDGeneral\QCDCtrlMsgs.h"

#include "resource.h"

extern HINSTANCE		hInstance;
extern HWND				hwndPlayer;
extern QCDModInitGen	*QCDCallbacks;

// Calls from the Player
void Configure(int flags);
void About(int flags);
void ShutDown(int flags);

// Other functions
void BrowseForFile();
void StartEvilLyrics();
void CloseEvilLyrics();
void MakeMenu();
HWND FindEvilLyrics();

// Subclassing
BOOL CALLBACK ConfigDialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
//LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif //QCDGeneralDLL_H