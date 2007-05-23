//-----------------------------------------------------------------------------
//
// File:	QCDModFileInfo
//
// About:	Tag Editing and File information module interface.  This file is 
//			published with the QCD plugin SDK.
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

#ifndef QCDMODFILEINFO_H
#define QCDMODFILEINFO_H

#include "QCDModDefs.h"

// name of the DLL export for output plugins
#define FILEINFO_DLL_ENTRY_POINT		PLUGIN_ENTRY_POINT(FileInfoModule)

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

//-----------------------------------------------------------------------------

struct QCDModInitFileInfo
{
	UINT				size;			// size of init structure
	UINT				version;		// plugin structure version (set to PLUGIN_API_VERSION)
	PluginServiceFunc	Service;		// player supplied services callback

	struct
	{
		int 	(*Initialize)(QCDModInfo *modInfo, int flags);
		void	(*ShutDown)(int flags);

		int 	(*ReadInfo)(const WCHAR* medianame, void* infoHandle, int flags);
		void*	reserved[4];

	} toModule;

	struct
	{
		int 	(*SetInfoInt)(void* infoHandle, const WCHAR* pszName, int nValue);
		int 	(*SetInfoString)(void* infoHandle, const WCHAR* pszName, const WCHAR* pszValue);
		void*	reserved[4];

	} toPlayer;
};


#endif //QCDMODFILEINFO_H