//-----------------------------------------------------------------------------
//
// File:	IQCDTagInfo
//
// About:	Tag Editing exportable interface.  This file is published with the
//			QCD plugin SDK.
//
// Authors:	Written by Paul Quinn
//
// Copyright:
//
//	QCD multimedia player application Software Development Kit
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

#ifndef IQCDTAGINFO_H
#define IQCDTAGINFO_H

//-----------------------------------------------------------------------------

#include "IQCDMediaInfo.h"
#include "QCDTagData.h"


//-----------------------------------------------------------------------------
// Tag plugin capabilities
#define TAGCAPS_READ				0x1
#define TAGCAPS_WRITE				0x2
#define TAGCAPS_STRIP				0x4
#define TAGCAPS_NTUTF8				0x1000
#define TAGCAPS_UNICODE				0x2000

//-----------------------------------------------------------------------------
// read/write/strip flag defines

#define TAG_DEFAULT			0x0
#define TAG_LEADING			0x1
#define TAG_TRAILING		0x2
#define TAG_ALL				(TAG_LEADING|TAG_TRAILING)

// <title IQCDTagInfo>
// 
// Provides access to tag metadata for media files. 

struct _declspec(novtable)
IQCDTagInfo
{
	// Increment reference count for interface
	virtual ULONG	__stdcall AddRef();
	// Decrement reference count on interface
	virtual ULONG	__stdcall Release();

	// Gets the supported capabilities provided for the current tagging format
	virtual int		__stdcall GetCaps();
	// Gets file name this interface is for
	virtual LPCWSTR __stdcall GetFileName();
	// Clears all data from interface
	virtual BOOL	__stdcall ClearFields();
	// Gets number of fields stored in interface
	virtual int		__stdcall GetFieldCount();

	// Read the metadata for the media file
	virtual BOOL	__stdcall ReadFromFile(int flags);
	// Writes the current fields to the media’s tag
	virtual BOOL	__stdcall WriteToFile(int flags);
	// Removes the media’s tag from the file
	virtual BOOL	__stdcall StripFromFile(int flags);

	// Sets value for tag field to the interface
	virtual BOOL	__stdcall SetTagDataByName(LPCWSTR pszName, QTAGDATA_TYPE type, const BYTE* pData, DWORD dwDataLen, int* startIndex);

	// Retrieves existing value for field name from interface
	virtual BOOL	__stdcall GetTagDataByName(LPCWSTR pszName, QTAGDATA_TYPE* pType, BYTE* pData, DWORD* pdwDataLen, int* startIndex);
	// Retrieves the existing value for the field, based on index
	virtual BOOL	__stdcall GetTagDataByIndex(int index, WCHAR* pszName, DWORD* pdwNameLen, QTAGDATA_TYPE* pType, BYTE* pData, DWORD* pdwDataLen);

	// Populate the tag interface with data from a IQCDMediaInfo interface
	virtual int		__stdcall SetTagDataFromMediaInfo(IQCDMediaInfo* pMediaInfo, int infoIndex, int flags);

	// Remove field based on index
	virtual BOOL	__stdcall RemoveTagDataByIndex(int index);
};

#endif //IQCDTAGINFO_H