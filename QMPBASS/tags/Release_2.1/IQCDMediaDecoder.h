//-----------------------------------------------------------------------------
//
// File:	IQCDMediaDecoder
//
// About:	Media Decoder exportable interface.  This file is published  
//			with the QCD plugin SDK.
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

#ifndef IQCDMEDIADECODER_H
#define IQCDMEDIADECODER_H

#include <windows.h>
#include <mmsystem.h>

//-----------------------------------------------------------------------------
// Common decoder defines
// 
// Each decoder implementation will interpret these flags as it sees fit, or
// possibly ignore them completely
//

#define MEDIADECODER_FLOAT_SAMPLES_YES		0x100	// decode to floating point samples if possible
#define MEDIADECODER_FLOAT_SAMPLES_NO		0x200	// do not decode to floating point samples if possible (integer samples)

//-----------------------------------------------------------------------------

struct _declspec(novtable)
IQCDMediaDecoderCallback
{
	// Receives decoded audio data
	virtual long	OnReceive(BYTE* buffer, int bufferSize, WAVEFORMATEX* wf, long userData);
	// Called at end of stream
	virtual void	OnEOF(long userData);
	// Called if there is an error decoding the stream
	virtual void	OnError(long error, long userData);
};

// <title IQCDMediaDecoder>
// 
// Provides access to decoded audio data for media files.  

struct _declspec(novtable)
IQCDMediaDecoder
{
	// Discards the interface
	virtual void	__stdcall Release();
	// Initiates decoding of media
	virtual BOOL	__stdcall StartDecoding(IQCDMediaDecoderCallback* pMDCallback, long userData);
};

#endif //IQCDMEDIADECODER_H