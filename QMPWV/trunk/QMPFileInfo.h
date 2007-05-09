//-----------------------------------------------------------------------------
// 
// File:	QMPFileInfo.h
//
// About:	QCD Player Input module DLL interface.  For more documentation, see
//			QCDModInput.h.
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

#ifndef QMPFileInfo_H
#define QMPFileInfo_H

#include "QCDModFileInfo.h"

#include "QMPModule.h"


//////////////////////////////////////////////////////////////////////////

class QMPFileInfo : public QMPModule< QMPFileInfo, QCDModInitFileInfo >
{
protected:
	friend class QMPModule< QMPFileInfo, QCDModInitFileInfo >;
	QMPFileInfo(void)
	{
		QCDCallbacks.version				= PLUGIN_API_VERSION_UNICODE;

		QCDCallbacks.toModule.Initialize	= Initialize;
		QCDCallbacks.toModule.ShutDown		= ShutDown;
		QCDCallbacks.toModule.ReadInfo		= ReadInfo;
	}

private:
	static BOOL Initialize(QCDModInfo *modInfo, int flags);
	static void ShutDown(int flags);

	static BOOL ReadInfo(const WCHAR* medianame, void* infoHandle, int flags);
};

#endif //QMPFileInfo_H

