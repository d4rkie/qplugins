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

#include "stdafx.h"
#include "QMPTAK.h"
#include "QMPFileInfo.h"

#include "QDecoder.h"


//..............................................................................
// Static Class Variables
QCDModInitFileInfo	QMPFileInfo::QCDCallbacks;

//-----------------------------------------------------------------------------

BOOL QMPFileInfo::Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = "TAK FileInfo";
	modInfo->moduleExtensions = "TAK";

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
	static WCHAR *chanmode[] = { L"N/A", L"Mono", L"Stereo", L"N/A", L"N/A", L"N/A", L"5.1 Surround" };

	WriteDataStruct wd;
	AudioInfo ai;
	CStringW wtext;

	QDecoder dec;
	QMediaReader mr((IQCDMediaSource *)QCDCallbacks.Service( opGetIQCDMediaSource, (void*)medianame, 0, 0));

	BOOL ret = FALSE;
	do {
		if ( !mr.Open())
			break;

		TrackExtents exts;
		if ( dec.GetTrackExtents( mr, exts))
			QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDDuration, (exts.end - exts.start) * 1.0 / exts.unitpersec * 1000);
		QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDByteSize, mr.GetSize());

		// reset file
		if ( !mr.Seek(0))
			break;

		if ( dec.Open( mr) <= 0)
			break;

		// decoding one frame to get information
		if ( !dec.Decode( wd))
			break;

		if ( !dec.GetAudioInfo( ai))
			break;

		QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDAvgBitrate, ai.bitrate);
		QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDSamplerate, wd.srate);
		QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDChannelCount, wd.nch);

		MBtoUCS2( ai.text, wtext);
		QCDCallbacks.toPlayer.SetInfoString( infoHandle, g_wszQCDFileFormat, wtext);
		QCDCallbacks.toPlayer.SetInfoString( infoHandle, g_wszQCDChannelMode, chanmode[wd.nch]);

		dec.Close();
		mr.Close();

		ret = TRUE;
	} while (0);

	// return true for successful read, false for failure
	return TRUE;
}

