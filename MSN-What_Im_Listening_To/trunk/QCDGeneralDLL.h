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
#include <IQCDMediaInfo.h>
#include "resource.h"

#include <QString.h>

//-----------------------------------------------------------------------------

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------

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


struct PlayingSongInfo
{
	int nCommand;     // 0-stopped, 1-playing
	
	int nLength;      // Song length
	int nTrackNumber;

	QString strType;  // Music, Video, Radio
	
	QString strTitle;
	QString strArtist;
	QString strAlbum;
	
	QString strYear;	
	QString strGenre;
	
	QString strRadioStationName;

	QString strWmContentId; // MSN specific
};

struct Settings
{
	BOOL bDebug;
	BOOL bTitle;
	BOOL bArtist;
	BOOL bAlbum;
	BOOL bVideo;
	BOOL bWMPIsFaked;
	UINT nDelay;
};


extern HINSTANCE    hInstance;
extern HWND         hwndPlayer;
extern QCDService*  QCDCallbacks;
extern Settings     settings;
extern BOOL         g_IsNewApi;


// Calls from the Player
int  Initialize(QCDModInfo *modInfo, int flags);
void Configure(int flags);
void About(int flags);
void ShutDown(int flags);


// Other
void LoadSettings();
void SaveSettings();

void CurrentSong(PlayingSongInfo* pSongInfo);
void UpdateSong(BOOL bSendBlogInfo);
void ClearSong();
void PlayPaused();

void SendSongInfo(PlayingSongInfo* pSongInfo);
BOOL SendToMSN(PlayingSongInfo* pSongInfo);
BOOL SendToMiranda(PlayingSongInfo* pSongInfo);

void StartTimer(UINT nForced = 0);

// Callbacks
void CALLBACK DelayTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);


// Subclassing
LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


// Config functions
BOOL CALLBACK ConfigDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif //QCDGeneralDLL_H