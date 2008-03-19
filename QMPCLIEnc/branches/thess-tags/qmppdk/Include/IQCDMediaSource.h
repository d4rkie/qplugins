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
// Creation flags

#define MEDIASOURCE_BUFFERING_YES	0x1		// use buffered reading
#define MEDIASOURCE_BUFFERING_NO	0x2		// no buffering (direct reading)

//-----------------------------------------------------------------------------
// OpenMedia flags

#define MEDIASOURCE_OPEN_FOR_PLAYBACK		0x0			// media is being read for playback
#define MEDIASOURCE_OPEN_FOR_SAVING			0x10		// media is being saved during playback
#define MEDIASOURCE_OPEN_FOR_ENCODING		0x20		// media is being read for encoding

//-----------------------------------------------------------------------------
// ReadBytes flags

#define MEDIASOURCE_READ_TRUMPPREBUFFER		0x1000		// do not wait for pre-buffering to complete to begin read
#define MEDIASOURCE_READ_NOPOSCHANGE		0x2000		// do not move read position with this read request

//-----------------------------------------------------------------------------
// SetMediaOptions flags

#define MEDIASOURCE_OPTION_BUFFERSIZE		0x100		// long (KB: 32 to 4096)
#define MEDIASOURCE_OPTION_PREBUFFERPERCENT	0x200		// long	(0 to 100)
#define MEDIASOURCE_OPTION_REBUFFERPERCENT	0x400		// long	(0 to 100)

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
// IQCDMediaSourceStatus status messages

#define MEDIASOURCE_STATUS_CONNECTING		1			// connecting to source media
#define MEDIASOURCE_STATUS_BUFFERING		2			// reading source media to fill pre-buffer
#define MEDIASOURCE_STATUS_DISCONNECTED		3			// disconnected from source media

#define MEDIASOURCE_STATUS_MINUTELIMIT		100			// duration played limit reached for source media
#define MEDIASOURCE_STATUS_TITLELIMIT		101			// titles played limit reached for source media
#define MEDIASOURCE_STATUS_CONNECTIONRETRY	110			// connection dropped, reconnect occurring

//-----------------------------------------------------------------------------
// error values

#define MSERROR_SUCCESS						0			// command successful
#define MSERROR_FAILED						-1			// command failed
#define MSERROR_NOTIMPLEMENTED				-2			// command not implemented by source plug-in
#define MSERROR_UNSUPPORTED					-3			// command not supported by source plug-in
#define MSERROR_SOURCE_NOTFOUND				-10			// media source was not located/connected to
#define MSERROR_SOURCE_BUFFERING			-11			// command failed as source was buffering (for ReadBytes)
#define MSERROR_SOURCE_EXHAUSTED			-12			// command failed as source was finished (for ReadBytes)
#define MSERROR_SOURCE_DISCONNECTED			-13			// command failed as source is not connected
#define MSERROR_SOURCE_UNSUPPORTED			-14			// command failed as source media is not supported
#define MSERROR_SOURCE_OPENDENIED			-15			// openning source was denied (DRM or unsupported MEDIASOURCE_OPEN_* flag)
#define MSERROR_UNEXPECTED					-20			// command was not expected at this time
#define MSERROR_INVALID_ARG					-21			// bad parameter passed to command
#define MSERROR_MEMORY						-22			// memory failure (out of memory?)

//-----------------------------------------------------------------------------

struct _declspec(novtable)
IQCDMediaSourceStatus
{
	virtual void	__stdcall StatusMessage(long statusFlag, LPCWSTR statusMsg, long userData);

	virtual long	__stdcall SetMetadataValue(LPCWSTR szValueName, LPCWSTR szValue, IQCDMediaInfo* pMetadata, long userData);
	virtual long	__stdcall SourceMetadataUpdate(IQCDMediaInfo* pMetadata, long userData);
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
	virtual bool	__stdcall IsConnected();

	virtual LPCWSTR	__stdcall GetMediaName();
	virtual long	__stdcall GetMediaProperties();

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