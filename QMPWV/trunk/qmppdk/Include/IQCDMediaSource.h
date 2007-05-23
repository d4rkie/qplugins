//-----------------------------------------------------------------------------
//
// File:	IQCDMediaSource
//
// About:	Media file reading and streaming exportable interface.  This file 
//			is published with the QCD plugin SDK.
//
// Authors:	Written by Paul Quinn
//
// Copyright:
//
//	QCD multimedia player application Software Development Kit
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

#ifndef IQCDMEDIASOURCE_H
#define IQCDMEDIASOURCE_H

#include <windows.h>
#include "IQCDMediaInfo.h"

//-----------------------------------------------------------------------------
// Media Properties flags

#define MEDIASOURCE_PROP_LOCALHARDDISK		0x1000
#define MEDIASOURCE_PROP_LOCALCDROM			0x2000
#define MEDIASOURCE_PROP_REMOVEABLEDISK		0x4000
#define MEDIASOURCE_PROP_NETWORKFILE		0x10000
#define MEDIASOURCE_PROP_INTERNETSTREAM		0x100000

#define MEDIASOURCE_PROP_CANSEEK			0x10
#define MEDIASOURCE_PROP_HASMETADATA		0x20

//-----------------------------------------------------------------------------
// Creation flags

#define MEDIASOURCE_BUFFERING_YES	0x1		// use buffered reading
#define MEDIASOURCE_BUFFERING_NO	0x2		// no buffering (direct reading)

//-----------------------------------------------------------------------------
// SetMediaOptions flags

#define MEDIASOURCE_OPTION_BUFFERSIZE		0x100		// long (KB: 32 to 4096)
#define MEDIASOURCE_OPTION_PREBUFFERPERCENT	0x200		// long	(0 to 100)
#define MEDIASOURCE_OPTION_REBUFFERPERCENT	0x400		// long	(0 to 100)

//-----------------------------------------------------------------------------
// ReadBytes flags

#define MEDIASOURCE_READ_TRUMPPREBUFFER		0x1000
#define MEDIASOURCE_READ_NOPOSCHANGE		0x2000

//-----------------------------------------------------------------------------
// IQCDMediaSourceStatus status messages

#define MEDIASOURCE_STATUS_CONNECTING		1
#define MEDIASOURCE_STATUS_BUFFERING		2
#define MEDIASOURCE_STATUS_DISCONNECTED		3

#define MEDIASOURCE_STATUS_RECEIVEDMETADATA	10

#define MEDIASOURCE_STATUS_MINUTELIMIT		100
#define MEDIASOURCE_STATUS_TITLELIMIT		101
#define MEDIASOURCE_STATUS_CONNECTIONRETRY	110

//-----------------------------------------------------------------------------
// error values

#define MSERROR_SUCCESS						0
#define MSERROR_FAILED						-1
#define MSERROR_NOTIMPLEMENTED				-2
#define MSERROR_UNSUPPORTED					-3
#define MSERROR_SOURCE_NOTFOUND				-10
#define MSERROR_SOURCE_BUFFERING			-11
#define MSERROR_SOURCE_EXHAUSTED			-12
#define MSERROR_SOURCE_DISCONNECTED			-13
#define MSERROR_SOURCE_UNSUPPORTED			-14
#define MSERROR_UNEXPECTED					-20
#define MSERROR_INVALID_ARG					-21
#define MSERROR_MEMORY						-22

//-----------------------------------------------------------------------------

struct _declspec(novtable)
IQCDMediaSourceStatus
{
	virtual void	__stdcall StatusMessage(long statusFlag, LPCWSTR statusMsg, long userData);
};

// <title IQCDMediaSource>
// 
// \ \                    

struct _declspec(novtable)
IQCDMediaSource
{
	virtual void	__stdcall Release();
	virtual long	__stdcall SetStatusCallback(IQCDMediaSourceStatus* pStatusCallback, long userData);

	virtual long	__stdcall OpenMedia(long flags);
	virtual long	__stdcall CloseMedia(long flags);

	virtual LPCWSTR	__stdcall GetMediaName();
	virtual long	__stdcall GetMediaProperties();
	virtual long	__stdcall GetAvailableMetadata(IQCDMediaInfo* pMediaInfo);

	virtual long	__stdcall SetOptionLong(long option, long value);
	virtual long	__stdcall GetOptionLong(long option);
	virtual long	__stdcall SetOptionString(long option, LPCWSTR value);
	virtual LPCWSTR	__stdcall GetOptionString(long option);

	virtual __int64	__stdcall GetSourceSize(long flags);
	virtual __int64	__stdcall GetSourcePosition(long flags);

	virtual long	__stdcall ReadBytes(BYTE* buffer, long bufferSize, long* bytesRead, long flags);
	virtual long	__stdcall SeekToByte(__int64 seekPos);
};

#endif //IQCDMEDIASOURCE_H