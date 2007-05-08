//-----------------------------------------------------------------------------
//
// File:	QCDModTagEditor2
//
// About:	Tag Editing plugin module interface.  This file is published with the 
//			QCD plugin SDK.
//
// Authors:	Written by Paul Quinn
//
// Copyright:
//
//	QCD multimedia player application Software Development Kit Release 1.0.
//
//	Copyright (C) 1997-2005 Quinnware
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

#ifndef QCDMODTAGEDITOR2_H
#define QCDMODTAGEDITOR2_H

#include "QCDModDefs.h"
#include "QCDTagData.h"

// name of the DLL export for output plugins
#define TAGEDITOR2_DLL_ENTRY_POINT			PLUGIN_ENTRY_POINT(TagEditorModule2)

//-----------------------------------------------------------------------------
// Tag plugin capabilities
#define TAGCAPS_READ				0x1
#define TAGCAPS_WRITE				0x2
#define TAGCAPS_STRIP				0x4
#define TAGCAPS_WANTUTF8			0x1000

//-----------------------------------------------------------------------------
// read/write/strip flag defines
#define TAG_DEFAULT			0x0
#define TAG_LEADING			0x1
#define TAG_TRAILING		0x2
#define TAG_ALL				(TAG_LEADING|TAG_TRAILING)

//-----------------------------------------------------------------------------

typedef struct _QCDModInitTag2
{
	UINT				size;			// size of init structure
	UINT				version;		// plugin structure version (set to PLUGIN_API_VERSION)
	PluginServiceFunc	Service;		// player supplied services callback

	struct
	{
		int 	(*Initialize)(QCDModInfo *modInfo, int flags);
		void	(*ShutDown)(int flags);

		void	(*Configure)(int flags);
		void	(*About)(int flags);

		int 	(*ReadFromFile)(const WCHAR* filename, void* tagHandle, int flags);
		int 	(*WriteToFile)(const WCHAR* filename, void* tagHandle, int flags);
		int 	(*StripFromFile)(const WCHAR* filename, void* tagHandle, int flags);
		void*	reserved[4];
	} toModule;

	struct
	{
		int 	(*SetTagData)(void* tagHandle, const WCHAR* pszName, QCD_TAGDATA_TYPE type, BYTE* pbData, DWORD dwDataLen, int* startIndex);

		int 	(*GetTagDataByName)(void* tagHandle, const WCHAR* pszName, QCD_TAGDATA_TYPE* pType, BYTE* pbData, DWORD* pdwDataLen, int* startIndex);
		int 	(*GetTagDataByIndex)(void* tagHandle, int index, WCHAR* pszName, DWORD* pdwNameLen, QCD_TAGDATA_TYPE* pType, BYTE* pData, DWORD* pdwDataLen);

		void*	reserved[4];
	} toPlayer;

} QCDModInitTag2;



#endif //QCDMODTAGEDITOR2_H