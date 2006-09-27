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
#include "resource.h"

//-----------------------------------------------------------------------------
// Old QCD entry point structure
//-----------------------------------------------------------------------------
	#define GENERALDLL_ENTRY_POINT	QGeneralModule // name of the DLL export for output plugins
	typedef struct 
	{
		UINT				size;			// size of init structure
		UINT				version;		// plugin structure version (set to PLUGIN_API_VERSION)
		PluginServiceFunc	Service;		// player supplied services callback

		struct
		{
			void *Reserved[2];
		} toPlayer;

		struct 
		{
			void (*ShutDown)(int flags);
			void (*Configure)(int flags);
			void (*About)(int flags);
			void *Reserved[2];
		} toModule;
	} QCDModInitGen;
//-----------------------------------------------------------------------------
typedef struct
{
	PluginServiceFunc	Service;		// player supplied services callback
} QCDService;


struct MSNMessages
{
	int msncommand;
	WCHAR title[100];
	WCHAR artist[100];
	WCHAR album[100];
	WCHAR wmcontentid[40];
};
struct Settings
{
	BOOL bTitle;
	BOOL bArtist;
	BOOL bAlbum;
	BOOL bVideo;
	BOOL bWMPIsFaked;
	UINT nDelay;
};


static const UINT PS_STOPPED = 1;
static const UINT PS_PLAYING = 2;
static const UINT PS_PAUSED  = 3;


extern HINSTANCE		hInstance;
extern HWND				hwndPlayer;
extern QCDService*      QCDCallbacks;
extern Settings			settings;
extern INT				nMSNBuild;

// Calls from the Player
int  Initialize(QCDModInfo *modInfo, int flags);
void Configure(int flags);
void About(int flags);
void ShutDown(int flags);

// Other
void LoadSettings();
void SaveSettings();
void CurrentSong(MSNMessages* msn);
void UpdateSong();
void ClearSong();
BOOL PlayerStatus(UINT status);

// Microsoft Messenger functins
void SendToMSN(MSNMessages* msn);
void StartTimer(UINT nForced = 0);

void RegDB_Fix(BOOL bFix);
void RegDB_Insert();
void RegDB_Clean();
INT  RegDB_GetWMPVersion();
INT  RegDB_GetMSNBuild();

// QBlog
void QBlog_InsertInRegDB();
void QBlog_CleanUpRegDB();

// Callbacks
void CALLBACK DelayTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);

// Subclassing
LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// Config functions
BOOL CALLBACK ConfigDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif //QCDGeneralDLL_H