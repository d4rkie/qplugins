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

#include "stdafx.h"
#include "QMPCUE.h"
#include "QMPFileInfo.h"

#include "IQCDFileInfo.h"

#include "QCUESheet.h"

//..............................................................................
// Static Class Variables
HWND				QMPFileInfo::hwndPlayer;
QCDModInitFileInfo	QMPFileInfo::QCDCallbacks;

//-----------------------------------------------------------------------------

BOOL QMPFileInfo::Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = (char *)L"CUE Sheet Virtual TrackInfo v1.0";
	modInfo->moduleExtensions = (char *)L"VT";

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
	// Parse/Process the CUE sheet file basing on the virtual track
	CPath pathImageFile;
	int vtNum;
	QCUESheet cueSheet;

	if ( !cueSheet.ReadFromVirtualTrack( (LPCTSTR)medianame))
		return FALSE;

	vtNum = cueSheet.GetVirtualTrackNumber();
	pathImageFile = cueSheet.GetImageFilePath( vtNum);

	// Import file infos from original image file.
	// NOTE: the duration field must be saved for fixing.
	//       So, this file infos are necessary.
	int duration = 0;
	IQCDFileInfo * piFI = (IQCDFileInfo *)QCDCallbacks.Service( opGetIQCDFileInfo, (char *)(LPCTSTR)pathImageFile, 0, 0);
	if ( !piFI)
		return FALSE;
	if ( !piFI->ReadInfo( flags)) {
		piFI->Release();
		return FALSE;
	}

	int index;
	int wnl;
	int value;
	int wvl;

	// Get int value by index
	index = 0;
	wnl = 0;
	value = 0;
	while ( piFI->GetIntByIndex( index++, NULL, &wnl, NULL)) {
		// skip the ZERO length file info field
		if ( wnl <= 0) continue;
		// get the information from the file info instance.
		++wnl;
		CStringW wn;
		// get the int value
		piFI->GetIntByIndex( index-1, wn.GetBuffer( wnl), &wnl, &value);
		wn.ReleaseBuffer();

		QCDCallbacks.toPlayer.SetInfoInt( infoHandle, wn, value);
		// save the duration field
		if ( wn == g_wszQCDDuration)
			duration = value;
	}

	// Get string value by index
	index = 0;
	wnl = 0;
	wvl = 0;
	while ( piFI->GetStringByIndex( index++, NULL, &wnl, NULL, &wvl)) {
		// skip the ZERO length file info field
		if ( wnl <=0 || wvl <= 0) continue;
		// get the information from the file info instance.
		++wnl; ++wvl;
		CStringW wn, wv;
		// get the string value
		piFI->GetStringByIndex( index-1, wn.GetBuffer( wnl), &wnl, wv.GetBuffer( wvl), &wvl);
		wn.ReleaseBuffer();
		wv.ReleaseBuffer();

		QCDCallbacks.toPlayer.SetInfoString( infoHandle, wn, wv);
	}

	piFI->Release();

	// fix the duration value
	duration = cueSheet.GetTrackEndIndex( vtNum, duration/1000.0) - cueSheet.GetTrackStartIndex( vtNum);
	QCDCallbacks.toPlayer.SetInfoInt( infoHandle, g_wszQCDDuration, duration);

	// return true for successful read, false for failure
	return TRUE;
}

