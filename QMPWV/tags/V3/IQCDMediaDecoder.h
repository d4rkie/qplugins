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

struct IQCDMediaDecoderCallback
{
	virtual long	OnReceive(BYTE* buffer, int bufferSize, WAVEFORMATEX* wf, long userData) = 0;
	virtual void	OnEOF(long userData) = 0;
	virtual void	OnError(long error, long userData) = 0;
};

//-----------------------------------------------------------------------------

struct IQCDMediaDecoder
{
	virtual void	__stdcall Release() = 0;
	virtual BOOL	__stdcall StartDecoding(IQCDMediaDecoderCallback* pMDCallback, long userData) = 0;
};

#endif //IQCDMEDIADECODER_H