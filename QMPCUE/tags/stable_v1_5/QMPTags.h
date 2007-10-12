//-----------------------------------------------------------------------------
// 
// File:	QMPTags.h
//
// About:	QCD Player Tag module DLL interface.  For more documentation, see
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

#ifndef QMPTags_H
#define QMPTags_H

#include "QCDModTagEditor2.h"

#include "QMPModule.h"


//////////////////////////////////////////////////////////////////////////

class QMPTags : public QMPModule< QMPTags, QCDModInitTag2 >
{
protected:
	friend class QMPModule< QMPTags, QCDModInitTag2 >;
	QMPTags(void)
	{
		QCDCallbacks.version				= PLUGIN_API_VERSION_UNICODE;

		QCDCallbacks.toModule.Initialize	= Initialize;
		QCDCallbacks.toModule.ShutDown		= ShutDown;
		QCDCallbacks.toModule.About			= About;
		QCDCallbacks.toModule.Configure		= Configure;
		QCDCallbacks.toModule.ReadFromFile	= ReadFromFile;
		QCDCallbacks.toModule.WriteToFile	= WriteToFile;
		QCDCallbacks.toModule.StripFromFile	= StripFromFile;
	}

private:
	static BOOL Initialize(QCDModInfo *modInfo, int flags);
	static void ShutDown(int flags);

	static BOOL ReadFromFile(LPCWSTR filename, void* tagHandle, int flags);
	static BOOL WriteToFile(LPCWSTR filename, void* tagHandle, int flags);
	static BOOL StripFromFile(LPCWSTR filename, void* tagHandle, int flags);

	static void About(int flags);
	static void Configure(int flags);
};

#endif //QMPTags_H

