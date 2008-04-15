//-----------------------------------------------------------------------------
// 
// File:	QCDFileInfo.h
//
// About:	QCD Player FileInfo module DLL interface.  For more documentation, see
//			QCDModFileInfo.h.
//
// Authors:	Written by Paul Quinn and Richard Carlson.
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

#ifndef QCDFileInfo_H
#define QCDFileInfo_H

#include "QCDModFileInfo.h"


extern QCDModInitFileInfo fiQCDCallbacks;

// Calls from the Player
BOOL fiInitialize(QCDModInfo *modInfo, int flags);
void fiShutDown(int flags);
BOOL fiReadInfo(const WCHAR* medianame, void* infoHandle, int flags);


#endif //QCDFileInfo_H
