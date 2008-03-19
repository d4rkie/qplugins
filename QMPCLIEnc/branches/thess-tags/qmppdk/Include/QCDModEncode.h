//-----------------------------------------------------------------------------
//
// File:	QCDModEncode.h
//
// About:	Encode plugin module interface.  This file is published with the 
//			Encode plugin SDK.
//
// Authors:	Written by Paul Quinn
//
// Copyright:
//
//	QCD multimedia player application Software Development Kit Release 1.0.
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

#ifndef QCDMODENCODE_H
#define QCDMODENCODE_H

#include "QCDModDefs.h"

// name of the DLL export for encode plugins
#define ENCODEDLL_ENTRY_POINT	PLUGIN_ENTRY_POINT(EncodeModule2)

// TestFormat return values
#define TESTFORMAT_ACCEPTED			0
#define TESTFORMAT_UNACCEPTED		-1
#define TESTFORMAT_FAILURE			-2
#define TESTFORMAT_NOTIMPL			-3

//-----------------------------------------------------------------------------

struct QCDModInitEnc
{
	UINT				size;			// size of init structure
	UINT				version;		// plugin structure version (set to PLUGIN_API_VERSION)
	PluginServiceFunc	Service;		// player supplied services callback

	struct
	{
		void *Reserved[16];
	}
	toPlayer;

	struct 
	{
		int  (*Initialize)(struct QCDModInfo *modInfo, int flags);			// initialize plugin
		void (*ShutDown)(int flags);

		int  (*Open)(LPCSTR outFile, LPCSTR srcFile, WAVEFORMATEX *wf, LPSTR openedFilename, int openedFilenameSize);
		int  (*Write)(struct WriteDataStruct* writeData);
		int  (*Drain)(int flags);
		int  (*Complete)(int flags);

		void (*Configure)(int flags);
		void (*About)(int flags);

		int  (*TestFormat)(WAVEFORMATEX *wf, int flags);

		void *Reserved[7];
	} 
	toModule;
};

#endif //QCDMODENCODE_H