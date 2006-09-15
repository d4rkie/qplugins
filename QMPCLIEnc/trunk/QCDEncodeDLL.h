//-----------------------------------------------------------------------------
// 
// File:	QCDEncodeDLL.h
//
// About:	QCD Player Output module DLL interface.  For more documentation, see
//			QCDModOutput.h.
//
// Authors:	Written by Paul Quinn
//
//	QCD multimedia player application Software Development Kit Release 1.0.
//
//	Copyright (C) 1997-2006 Quinnware
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

#include "QCDModEncode.h"

#ifndef QCDENCODEDLL_H
#define QCDENCODEDLL_H

extern HINSTANCE		hInstance;
extern QCDModInitEnc	QCDCallbacks;

// Calls from the Player
void ShutDown(int flags);
BOOL Initialize(QCDModInfo *modInfo, int flags);

BOOL Open(LPCSTR outFile, LPCSTR srcFile, WAVEFORMATEX *wf, LPSTR openedFilename, int openedFilenameSize);
BOOL Write(WriteDataStruct* wd);
BOOL Drain(int flags);
BOOL Complete(int flags);

void Configure(int flags);
void About(int flags);

int  TestFormat(WAVEFORMATEX *wf, int flags);

#endif //QCDOUTPUTDLL_H