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

#include <QCDModGeneral2.h>
#include <QCDCtrlMsgs.h>
#include "SnarlInterface.h"

struct Settings
{
	UINT nTimeout;
	BOOL bCascade;
	BOOL bHeadline_wrap;
	//bool bText1_wrap;
	//bool bText2_wrap;

	BOOL bDisplayCoverArt;
	QString strCoverArtRoot;
	QString strCoverArtTemplate;

	PluginServiceOp Headline_ServiceOp;
	PluginServiceOp Text1_ServiceOp;
	PluginServiceOp Text2_ServiceOp;
	
};

extern HINSTANCE       hInstance;
extern HWND            hwndPlayer;
extern QCDModInitGen2  QCDCallbacks;
extern Settings        settings;

extern void            DisplaySongInfo();
extern bool            IsPlaying();

extern SnarlInterface* snarl;

// Calls from the Player
int Initialize(QCDModInfo *modInfo, int flags);
void Configure(int flags);
void About(int flags);
void ShutDown(int flags);

// Helper functions
void LoadSettings();
void SaveSettings();
void DisplaySongInfo();
bool IsPlaying();
void GetCoverArt(long nIndex, LPSTR strIcon);
//void GetIcon2(long nIndex, LPSTR strIcon);
void Test();

// Subclassing
LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif //QCDGeneralDLL_H