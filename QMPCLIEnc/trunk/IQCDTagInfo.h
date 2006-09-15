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
#define TAGCAPS_WANTUTF8			0x1000

//-----------------------------------------------------------------------------
// read/write/strip flag defines

#define TAG_DEFAULT			0x0
#define TAG_LEADING			0x1
#define TAG_TRAILING		0x2
#define TAG_ALL				(TAG_LEADING|TAG_TRAILING)

//-----------------------------------------------------------------------------

struct IQCDTagInfo
{
	virtual void	__stdcall Release() = 0;
	virtual int		__stdcall GetCaps() = 0;
	virtual BOOL	__stdcall ClearFields() = 0;
	virtual int		__stdcall GetFieldCount() = 0;

	virtual BOOL	__stdcall ReadFromFile(int flags) = 0;
	virtual BOOL	__stdcall WriteToFile(int flags) = 0;
	virtual BOOL	__stdcall StripFromFile(int flags) = 0;

	virtual BOOL	__stdcall SetTagDataByName(LPCWSTR pszName, QCD_TAGDATA_TYPE type, const BYTE* pData, DWORD dwDataLen, int* startIndex) = 0;

	virtual BOOL	__stdcall GetTagDataByName(LPCWSTR pszName, QCD_TAGDATA_TYPE* pType, BYTE* pData, DWORD* pdwDataLen, int* startIndex) = 0;
	virtual BOOL	__stdcall GetTagDataByIndex(int index, WCHAR* pszName, DWORD* pdwNameLen, QCD_TAGDATA_TYPE* pType, BYTE* pData, DWORD* pdwDataLen) = 0;

	virtual int		__stdcall SetTagDataFromMediaInfo(IQCDMediaInfo* pMediaInfo, int infoIndex, int flags) = 0;
};

#endif //IQCDTAGINFO_H