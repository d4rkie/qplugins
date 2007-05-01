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

#define STATUS_NOMARK	0
#define STATUS_MARKED	1
#define STATUS_ABMODE	2

#define IDM_ABREPEAT 40097


#include <QCDCtrlMsgs.h>
#include <QCDModDefs.h>
#include <QCDModGeneral.h>
#include <QCDModGeneral2.h>

#define IPC_GETOUTPUTTIME 105

typedef struct
{
	PluginServiceFunc	Service;		// player supplied services callback
} QCDService;

struct _Settings
{
	BOOL bShowWindowOnStart;
};

extern HINSTANCE		hInstance;
extern HWND				hwndPlayer;
extern QCDService*      QCDCallbacks;

extern HWND             hwndMainDlg;
extern HWND             hwndQCDWA;
extern _Settings        Settings;

extern BOOL m_bRunning;
extern LONG m_iStartPos, m_iEndPos;
extern BYTE m_Status;

// Calls from the Player
int  Initialize(QCDModInfo* modInfo, int flags);
void Configure(int flags);
void About(int flags);
void ShutDown(int flags);

// Callbacks
LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif //QCDGeneralDLL_H