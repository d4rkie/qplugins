//-----------------------------------------------------------------------------
//
// File:	QCDFileInfo.cpp
//
// About:	See QCDFileInfo.h
//
// Authors:	Written by Paul Quinn and Richard Carlson.
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

#include "QCDBass.h"
#include "QCDFileInfo.h"
#include <shlwapi.h>

// globals
extern std::string strAllExtensions;
QCDModInitFileInfo fiQCDCallbacks;

//-----------------------------------------------------------------------------

PLUGIN_API QCDModInitFileInfo* FILEINFO_DLL_ENTRY_POINT()
{
	fiQCDCallbacks.size					= sizeof(QCDModInitFileInfo);
	fiQCDCallbacks.version				= PLUGIN_API_VERSION_UNICODE;
	fiQCDCallbacks.toModule.Initialize	= fiInitialize;
	fiQCDCallbacks.toModule.ShutDown	= fiShutDown;
	fiQCDCallbacks.toModule.ReadInfo	= fiReadInfo;

	return &fiQCDCallbacks;
}

//-----------------------------------------------------------------------------

BOOL fiInitialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = (char*)L"QPlug-ins Bass FileInfo";
	modInfo->moduleExtensions = (char *)strAllExtensions.c_str();
	modInfo->moduleCategory = (char*)L"AUDIO";

	return TRUE;
}

//-----------------------------------------------------------------------------

void fiShutDown(int flags)
{
	// TODO:
	// prepare plugin to be unloaded. All allocations should be freed.
}

//-----------------------------------------------------------------------------

BOOL fiReadInfo(const WCHAR* medianame, void* infoHandle, int flags)
{
	static WCHAR *chanmode[] = { L"N/A", L"Mono", L"Stereo", L"N/A", L"N/A", L"N/A", L"5.1 Surround" };

	HSTREAM hStream = BASS_StreamCreateFile(FALSE, medianame, 0, 0, BASS_STREAM_DECODE|BASS_UNICODE);
	if (!hStream)
		return FALSE;

	float seconds = (float)BASS_ChannelBytes2Seconds(hStream, BASS_ChannelGetLength(hStream, BASS_POS_BYTE));

	fiQCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDDuration, (int)(seconds * 1000.f));
	fiQCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDByteSize, (int)BASS_ChannelGetLength(hStream, BASS_POS_BYTE));
	// See bass::get_bitrate() for the following
	//fiQCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDAvgBitrate, 0);

	BASS_CHANNELINFO info;
	if ( BASS_ChannelGetInfo(hStream, &info)) {	
		fiQCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDSamplerate, info.freq);
		fiQCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDChannelCount, info.chans);
	}

	fiQCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDFrameOffset, (int)BASS_ChannelGetPosition(hStream, BASS_POS_BYTE));

	fiQCDCallbacks.toPlayer.SetInfoString( infoHandle, g_wszQCDFileFormat, PathFindExtensionW(medianame) + 1);
	fiQCDCallbacks.toPlayer.SetInfoString( infoHandle, g_wszQCDChannelMode, info.chans < 2 ? L"Mono" : (info.chans > 2 ? L"Multi-Channel" : L"Stereo"));

	BASS_StreamFree(hStream);

	// return true for successful read, false for failure
	return TRUE;
}

