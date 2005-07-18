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

#ifndef QCDMODENCODE_H
#define QCDMODENCODE_H

#include "QCDModDefs.h"

// name of the DLL export for encode plugins
#define ENCODEDLL_ENTRY_POINT	QEncodeModule2

//-----------------------------------------------------------------------------

typedef struct 
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
		int  (*Initialize)(QCDModInfo *modInfo, int flags);			// initialize plugin
		void (*ShutDown)(int flags);

		BOOL (*Open)(LPCSTR outFile, LPCSTR srcFile, WAVEFORMATEX *wf, LPSTR openedFilename, int openedFilenameSize);
		BOOL (*Write)(WriteDataStruct*);
		BOOL (*Drain)(int flags);
		BOOL (*Complete)(int flags);

		void (*Configure)(int flags);
		void (*About)(int flags);

		void *Reserved[8];
	} 
	toModule;

} QCDModInitEnc;

#endif //QCDMODENCODE_H