//-----------------------------------------------------------------------------
//
// File:	QMPFileInfo.cpp
//
// About:	See QMPFileInfo.h
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

#include "stdafx.h"
#include "QMPWV.h"
#include "QMPFileInfo.h"

#include "wavpack.h"

//..............................................................................
// Static Class Variables
HWND				QMPFileInfo::hwndPlayer;
QCDModInitFileInfo	QMPFileInfo::QCDCallbacks;

//-----------------------------------------------------------------------------

BOOL QMPFileInfo::Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = "WavPack FileInfo";
	modInfo->moduleExtensions = "WV";

	return TRUE;
}

//-----------------------------------------------------------------------------

void QMPFileInfo::ShutDown(int flags)
{
	// TODO:
	// prepare plugin to be unloaded. All allocations should be freed.
}

//-----------------------------------------------------------------------------

BOOL QMPFileInfo::ReadInfo(const WCHAR* medianame, void* infoHandle, int flags)
{
	CStringA filename;
	WavpackContext * wpc;
	char error [128] = {'\0'};
	int duration;
	int nch;

	UCS2toMB( medianame, filename);

	if ( !(wpc = WavpackOpenFileInput( filename, error, (g_bUseWVC & OPEN_WVC) | OPEN_2CH_MAX, 0)))
		return FALSE;

	duration = (int)((int)(WavpackGetNumSamples( wpc) * 1000.0 / WavpackGetSampleRate( wpc)));
	nch = WavpackGetNumChannels( wpc);

	QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDDuration, duration);
	QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDByteSize, WavpackGetFileSize( wpc));
	QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDAvgBitrate, (int)WavpackGetAverageBitrate( wpc, 0));
	QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDSamplerate, WavpackGetSampleRate( wpc));
	QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDChannelCount, nch);
	//QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDFrameOffset, wpc->filepos);

	QCDCallbacks.toPlayer.SetInfoString( infoHandle, g_wszQCDFileFormat, L"WavPack");
	QCDCallbacks.toPlayer.SetInfoString( infoHandle, g_wszQCDChannelMode, nch < 2 ? L"Mono" : (nch > 2 ? L"Multi-Channel" : L"Stereo"));

	wpc = WavpackCloseFile( wpc);

	// return true for successful read, false for failure
	return TRUE;
}

