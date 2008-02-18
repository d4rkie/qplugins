//-----------------------------------------------------------------------------
// 
// SDK Authors:	Written by Paul Quinn and Richard Carlson.
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

#ifndef QMPGeneralDLL_H
#define QMPGeneralDLL_H

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <QString.h>

#include <crtdbg.h>
#ifdef _DEBUG
	#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif



#include <TCHAR.H>
#include <QCDModGeneral2.h>
#include <QCDCtrlMsgs.h>


#include "resource.h"

//----------------------------------------------------------------------------

struct _Settings {
	QString strNowPlayingPath;
	QString strXmlExportPath;
	QString strHtmlExportPath;
};




//----------------------------------------------------------------------------
// Extern declarations
extern HINSTANCE       g_hInstance;
extern HWND            g_hwndPlayer;
extern QCDModInitGen2  QMPCallbacks;

extern _Settings Settings;


//----------------------------------------------------------------------------
// Calls from the Player
int  Initialize(QCDModInfo* modInfo, int flags);
void Configure(int flags);
void About(int flags);
void ShutDown(int flags);


#endif //QMPGeneralDLL_H