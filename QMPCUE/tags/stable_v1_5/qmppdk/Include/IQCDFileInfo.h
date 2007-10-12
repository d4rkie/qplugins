//-----------------------------------------------------------------------------
//
// File:	IQCDFileInfo
//
// About:	File information exportable interface.  This file is published  
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

#ifndef IQCDFILEINFO_H
#define IQCDFILEINFO_H

//-----------------------------------------------------------------------------

// Defined file info fields

// integer values
#define g_wszQCDDuration		L"Duration"			// in milliseconds
#define g_wszQCDByteSize		L"ByteSize"			// in bytes
#define g_wszQCDAvgBitrate		L"AvgBitrate"		// full bitrate value (128kbps = 128000)
#define g_wszQCDSamplerate		L"Samplerate"		// full sameplate value (44.1khz = 44100)
#define g_wszQCDChannelCount	L"ChannelCount"		// number of channels (1 = mono, 2 = stereo)
#define g_wszQCDFrameOffset		L"FrameOffset"		// byte offset to first valid frame

#define g_wszQCDWidth			L"Width"			// native width
#define g_wszQCDHeight			L"Height"			// native heigth

// string values
#define g_wszQCDBitrateType		L"BitrateType"		// "CBR" or "VBR" for example
#define g_wszQCDFileFormat		L"FileFormat"		// "MP3" for example
#define g_wszQCDChannelMode		L"ChannelMode"		// "Stereo" for example
#define g_wszQCDProtectionType	L"ProtectionType"	// "WM DRM v9" for example

#define g_wszQCDFrameRate		L"FrameRate"		// framerate (Hz)
#define g_wszQCDAspectRatio		L"AspectRatio"		// 16:9, 4:3, etc

// <title IQCDFileInfo>
// 
// Provides access to intrinsic details about media files.

struct _declspec(novtable)
IQCDFileInfo
{
	// Discards the interface
	virtual void	__stdcall Release();

	// Load the file information
	virtual BOOL	__stdcall ReadInfo(int flags);

	// Retrieves the named integer value from the interface
	virtual BOOL	__stdcall GetIntByName(LPCWSTR pszName, int *pValue);
	// Retrieves the integer value from the interface based on its order
	virtual BOOL	__stdcall GetIntByIndex(int index, WCHAR* pszName, int* pnNameLen, int* pValue);

	// Retrieves the named string value from the interface
	virtual BOOL	__stdcall GetStringByName(LPCWSTR pszName, WCHAR* pValue, int* pnValueLen);
	// Retrieves the string value from interface based on its order
	virtual BOOL	__stdcall GetStringByIndex(int index, WCHAR* pszName, int* pnNameLen, WCHAR* pValue, int* pnValueLen);
};

#endif //IQCDFILEINFO_H