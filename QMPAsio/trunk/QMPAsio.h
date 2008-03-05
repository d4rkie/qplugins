//-----------------------------------------------------------------------------
// 
// File:	QMPAsio.h
//
// About:	QCD Player playback module DLL interface.  For more documentation, 
//			see QCDModPlayback2.h
//
// Authors:	Adapted for ASIO interface by Ted Hess
//
//	QMP multimedia player application Software Development Kit Release 5.0.
//
//	Copyright (C) 1997-2007 Quinnware
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "mmreg.h"
#include "QCDModPlayback2.h"
#include "resource.h"		// main symbols

#include "AsioHost.h"
#include "QMPAsioConfig.h"
#include "QMPAsioAbout.h"

int Initialize(QCDModInfo *ModInfo, int flags);
void ShutDown(int flags);

BOOL Open(LPCSTR file, WAVEFORMATEX *wf);
BOOL Write(WriteDataStruct *wf);
BOOL Flush(UINT marker);
BOOL Stop(int flags);
BOOL Pause(int flags);
BOOL Drain(int flags);
BOOL DrainCancel(int flags);
BOOL SetVolume(int levelleft, int levelright, int flags);
BOOL GetCurrentPosition(UINT *position, int flags);

void Configure(int flags);
void About(int flags);

// CQMPAsioApp
// See QMPAsio.cpp for the implementation of this class
//

class CQMPAsioApp : public CWinApp
{
public:
	CQMPAsioApp();

// Overrides
public:
	virtual BOOL InitInstance();
	void LoadResString(UINT uID, LPTSTR lpBuffer, int nBufferMax);

	DECLARE_MESSAGE_MAP()

public:
	HWND			m_hQCDWin;

	QCDModInitPlay2	*m_pQCDCallbacks;
	CAsioHost		*m_pAsioHost;

	CQMPAsioAbout	*m_pAbout;
	CQMPAsioConfig	*m_pConfig;
	AsioDrivers		*m_pAsioDrivers;

	DWORD			m_nPrefPageID;

	DWORD			m_nPosition;
};

extern CQMPAsioApp	asioApp;
