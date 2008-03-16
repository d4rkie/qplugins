//-----------------------------------------------------------------------------
//
// File:	QMPPlaylist.cpp
//
// About:	See QMPPlaylist.h
//
// Authors:	Written by Paul Quinn
//
//	QCD multimedia player application Software Development Kit Release 1.0.
//
//	Copyright (C) 1997-2003 Quinnware
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
#include "QMPPlaylists.h"

#include "QCUESheet.h"

#include "AboutDlg.h"
//#include "GenerateCUESheetDlg.h"

//-----------------------------------------------------------------------------
// Static Class Variables
QCDModInitPL	QMPPlaylists::QCDCallbacks;

WCHAR g_szPlaylistModDisplayStr[1024] = {0};

typedef struct {
	QCUESheet * cueSheet;
	FILE * fpOut;
	BOOL bJustCreated;
	UINT uLastTrackNum;
	INT iLastEndIndex;
} PLInstanceData;

//CGenerateCUESheetDlg g_dlgGenerateCUESheet;

//-----------------------------------------------------------------------------

BOOL QMPPlaylists::Initialize(QCDModInfo *modInfo, int flags)
{
	modInfo->moduleString = (LPSTR)g_szPlaylistModDisplayStr;
	modInfo->moduleExtensions = (LPSTR)L"CUE";

	// load display name
	ResInfo resInfo = { sizeof(ResInfo), g_hInstance, MAKEINTRESOURCE(IDS_PLAYLIST_MODULE), 0, 0 };
	QCDCallbacks.Service(opLoadResString, (void*)g_szPlaylistModDisplayStr, (long)sizeof(g_szPlaylistModDisplayStr), (long)&resInfo);

	return TRUE;
}

//-----------------------------------------------------------------------------

void QMPPlaylists::ShutDown(int flags)
{
}

//-----------------------------------------------------------------------------

void* QMPPlaylists::InitPlaylist(const char* playlist, int inflags, int *outflags)
{
	// QCD will call InitPlaylist to begin any interaction with a playlist
	// In this routine you must create an instance of any data that the other
	// functions may require

	// create instance of your playlist instance struct
	PLInstanceData* plhandle = (PLInstanceData*)GlobalAlloc(GPTR, sizeof(PLInstanceData));

	if ( plhandle) {
		// tell player what you require
		if (outflags)
			*outflags = PLAYLIST_WANT_ALL;

		// Initialize all
		plhandle->cueSheet = NULL;
		plhandle->fpOut = NULL;
		plhandle->bJustCreated = FALSE;
		plhandle->uLastTrackNum = 0;
		plhandle->iLastEndIndex = -1;

		if ( inflags & PLAYLIST_INIT_READ) {
			// init instance struct
			plhandle->cueSheet = new QCUESheet;
			if ( !plhandle->cueSheet || !plhandle->cueSheet->ReadFromCUEFile( (LPCWSTR)playlist))
				return NULL;
		} else if ( inflags & PLAYLIST_INIT_WRITE) {
			if ( 0 != _tfopen_s( &plhandle->fpOut, (LPCTSTR)playlist, _T("w")))
				return NULL;
			plhandle->bJustCreated  = TRUE;
		} else if ( inflags & PLAYLIST_INIT_APPEND) {
			plhandle->cueSheet = new QCUESheet;
			if ( !plhandle->cueSheet || !plhandle->cueSheet->ReadFromCUEFile( (LPCWSTR)playlist))
				return NULL;
			plhandle->uLastTrackNum = plhandle->cueSheet->GetNumTracks();
			if ( 0 != _tfopen_s( &plhandle->fpOut, (LPCTSTR)playlist, _T("a")))
				return NULL;
		} else {
			return NULL;
		}
	}

	return plhandle;
}

int QMPPlaylists::LoadEntries(void* plHandle, void* _plData, int flags)
{
	PLInstanceData* plData = (PLInstanceData*)_plData;

	// insert individual virtual tracks into playlist window
	for ( unsigned int i = 1; i <= plData->cueSheet->GetNumTracks(); ++i) {
		PLEntryData	entryData;
		CStringW vtname;

		entryData.struct_size = sizeof(PLEntryData);
		entryData.mediaType = DIGITAL_AUDIOFILE_MEDIA;
		entryData.extentsValid = FALSE;
		{
			// actually, we do not care about this
			entryData.extentsValid = TRUE;
			entryData.trackExtents.track = i;
			entryData.trackExtents.unitpersec = 1000;
			entryData.trackExtents.start = plData->cueSheet->GetTrackStartIndex( i) * entryData.trackExtents.unitpersec;
			entryData.trackExtents.end = plData->cueSheet->GetTrackEndIndex( i, 0.0) * entryData.trackExtents.unitpersec;
			entryData.trackExtents.bytesize = 0;
		}
		entryData.title = (char *)(LPCTSTR)(plData->cueSheet->GetTrackTagByName( i, QCDTag_Title));
		entryData.album = (char *)(LPCTSTR)(plData->cueSheet->GetTrackTagByName( i, QCDTag_Album));
		entryData.artist = (char *)(LPCTSTR)(plData->cueSheet->GetTrackTagByName( i, QCDTag_Artist));

		vtname.Format( L"%s,%d.vt", plData->cueSheet->GetCUESheetFilePath(), i);

		QCDCallbacks.toPlayer.LoadEntry( plHandle, (LPSTR)(LPCWSTR)vtname, &entryData, 0);
	}

	QCDCallbacks.toPlayer.LoadComplete( plHandle, 0);

	return TRUE;
}

//-----------------------------------------------------------------------------

int	QMPPlaylists::SaveEntry(void* plData, const char* medianame, PLEntryData* plEntryData, int flags)
{
	PLInstanceData* plhandle = (PLInstanceData*)plData;
	CString str;

	CPath pathImageFile;
	int vtNum;
	QCUESheet cueSheet;
	BOOL isVTrack = FALSE;

	// Output Header on writing
	if ( plhandle->bJustCreated) {
		// Output Album title
		_fputts( _T("TITLE \"Misc\"\n"), plhandle->fpOut);
		// Output Album artist
		_fputts( _T("PERFORMER \"Various Artists\"\n"), plhandle->fpOut);

		plhandle->bJustCreated = FALSE;
	}

	// Output File
	if ( plhandle->iLastEndIndex < 0) {
		str = _T("FILE \"");
		CString ext = PathFindExtension( (LPCTSTR)medianame);
		if ( lstrcmpi( ext, _T(".vt")) == 0) { // NOTE! It's a virtual track
			if ( !cueSheet.ReadFromVirtualTrack( (LPCTSTR)medianame))
				return TRUE; // skip

			vtNum = cueSheet.GetVirtualTrackNumber();
			pathImageFile = cueSheet.GetImageFilePath(vtNum);

			str += pathImageFile;
			ext = pathImageFile.GetExtension(); // direct to new image file
			isVTrack = TRUE;
		} else {
			str += (LPCTSTR)medianame;
		}
		str += _T("\"");
		if ( lstrcmpi( ext, _T(".mp3")) == 0)
			str += _T(" MP3");
		else if ( lstrcmpi( ext, _T(".aiff")) == 0)
			str += _T(" AIFF");
		else
			str += _T(" WAVE");
		str += _T("\n");
		_fputts( str, plhandle->fpOut);
	}

	// Output Track
	str.Format( _T("  TRACK %02d AUDIO\n"), ++plhandle->uLastTrackNum);
	_fputts( str, plhandle->fpOut);

	// Output Track title
	str = _T("    TITLE \"");
	str += (LPCTSTR)plEntryData->title;
	str += _T("\"\n");
	_fputts( str, plhandle->fpOut);

	// Output Track artist
	str = _T("    PERFORMER \"");
	str += (LPCTSTR)plEntryData->artist;
	str += _T("\"\n");
	_fputts( str, plhandle->fpOut);

	// Output Index 00 for gap
	if ( plhandle->iLastEndIndex > 0) {
		str.Format( _T("    INDEX 00 %02d:%02d:%02d\n"), (plhandle->iLastEndIndex/75)/60, (plhandle->iLastEndIndex/75)%60, plhandle->iLastEndIndex%75);
		_fputts( str, plhandle->fpOut);

		plhandle->iLastEndIndex = -1;

		str = _T("FILE \"");
		CString ext = PathFindExtension( (LPCTSTR)medianame);
		if ( lstrcmpi( ext, _T(".vt")) == 0) { // NOTE! It's a virtual track
			if ( !cueSheet.ReadFromVirtualTrack( (LPCTSTR)medianame))
				return TRUE; // skip

			vtNum = cueSheet.GetVirtualTrackNumber();
			pathImageFile = cueSheet.GetImageFilePath(vtNum);

			str += pathImageFile;
			ext = pathImageFile.GetExtension(); // direct to new image file
			isVTrack = TRUE;
		} else {
			str += (LPCTSTR)medianame;
		}
		str += _T("\"");
		if ( lstrcmpi( ext, _T(".mp3")) == 0)
			str += _T(" MP3");
		else if ( lstrcmpi( ext, _T(".aiff")) == 0)
			str += _T(" AIFF");
		else
			str += _T(" WAVE");
		str += _T("\n");
		_fputts( str, plhandle->fpOut);
	}

	// Output Index 01 for start
	if ( isVTrack) {
		int start = cueSheet.GetTrackStartIndexFrame( vtNum);
		int end = cueSheet.GetTrackEndIndexFrame( vtNum);

		str.Format( _T("    INDEX 01 %02d:%02d:%02d\n"), (start/75)/60, (start/75)%60, start%75);
		if ( end > 0 && end > start)
			plhandle->iLastEndIndex = end;
	} else {
		str = _T("    INDEX 01 00:00:00\n");
	}
	_fputts( str, plhandle->fpOut);

	return TRUE;
}

//-----------------------------------------------------------------------------

void QMPPlaylists::DeinitPlaylist(void* plData, int flags)
{
	// Playlist interaction is complete
	// Clean up (close any files, etc)

	PLInstanceData* plhandle = (PLInstanceData*)plData;

	if ( plhandle->cueSheet) delete plhandle->cueSheet;
	if ( plhandle->fpOut) fclose( plhandle->fpOut);

	GlobalFree(plData);
}

//-----------------------------------------------------------------------------

void QMPPlaylists::Configure(int flags)
{
	//
	// TODO : Show "configure" dialog.
	//
}

//-----------------------------------------------------------------------------

void QMPPlaylists::About(int flags)
{
	CAboutDlgPlaylists dlg;
	dlg.DoModal( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0));
}

