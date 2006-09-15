//-----------------------------------------------------------------------------
//
// File:	IQCDMediaInfo
//
// About:	Metadata information for media exportable interface.  This file 
//			is published with the QCD plugin SDK.
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

#ifndef IQCDMEDIAINFO_H
#define IQCDMEDIAINFO_H

//-----------------------------------------------------------------------------

// Defined info fields

#define		QCDInfo_ArtistAlbum				L"ArtistAlbum"
#define		QCDInfo_ArtistTrack				L"ArtistTrack"
#define		QCDInfo_TitleAlbum				L"TitleAlbum"
#define		QCDInfo_TitleTrack				L"TitleTrack"
#define		QCDInfo_TrackNumber				L"TrackNumber"
#define		QCDInfo_DiscNumber				L"DiscNumber"
#define		QCDInfo_GenreAlbum				L"GenreAlbum"
#define		QCDInfo_GenreAlbum2				L"GenreAlbum2"
#define		QCDInfo_GenreTrack				L"GenreTrack"
#define		QCDInfo_GenreTrack2				L"GenreTrack2"
#define		QCDInfo_YearAlbum				L"YearAlbum"
#define		QCDInfo_YearTrack				L"YearTrack"

#define		QCDInfo_AlbumTUID				L"AlbumTUID"
#define		QCDInfo_AlbumCDID				L"AlbumCDID"
#define		QCDInfo_TrackFileID				L"TrackFileID"
#define		QCDInfo_TrackExtData			L"TrackExtData"
#define		QCDInfo_AlbumISRC				L"AlbumISRC"
#define		QCDInfo_TrackISRC				L"TrackISRC"
#define		QCDInfo_AlbumTOC				L"AlbumTOC"
#define		QCDInfo_AlbumCompilation		L"AlbumCompilation"
#define		QCDInfo_AlbumNumTracks			L"AlbumNumTracks"
#define		QCDInfo_AlbumHasAudio			L"AlbumHasAudio"

#define		QCDInfo_Publisher				L"Publisher"
#define		QCDInfo_Comment					L"Comment"
#define		QCDInfo_Composer				L"Composer"
#define		QCDInfo_Conductor				L"Conductor"
#define		QCDInfo_Orchestra				L"Orchestra"
#define		QCDInfo_Lyricist				L"Lyricist"
#define		QCDInfo_OriginalArtist			L"OrigArtist"
#define		QCDInfo_Copyright				L"Copyright"
#define		QCDInfo_EncodedBy				L"EncodedBy"
#define		QCDInfo_Mood					L"Mood"
#define		QCDInfo_DurationMS				L"DurationMS"
#define		QCDInfo_Rating					L"Rating"
#define		QCDInfo_ByteSize				L"ByteSize"
#define		QCDInfo_Bitrate					L"Bitrate"
#define		QCDInfo_OrigArtist				L"OrigArtist"
#define		QCDInfo_SampleRate				L"SampleRate"
#define		QCDInfo_Channels				L"Channels"
#define		QCDInfo_BPM						L"BPM"

#define		QCDInfo_ReplayGain_Album_Gain	L"ReplayGain_Album_Gain"
#define		QCDInfo_ReplayGain_Track_Gain	L"ReplayGain_Track_Gain"
#define		QCDInfo_ReplayGain_Album_Peak	L"ReplayGain_Album_Peak"
#define		QCDInfo_ReplayGain_Track_Peak	L"ReplayGain_Track_Peak"

// defined flags for SetInfoByName

#define MEDIAINFO_ALWAYSSET			0x0
#define MEDIAINFO_SETIFEMPTY		0x10
#define MEDIAINFO_SETIFNOTEMPTY		0x20


//-----------------------------------------------------------------------------


struct IQCDMediaInfo
{
	virtual ULONG	__stdcall AddRef() = 0;
	virtual void	__stdcall Release() = 0;

	virtual LPCWSTR __stdcall GetMediaName() = 0;				// retrieves media name
	virtual long	__stdcall GetMediaType() = 0;				// file, cd, stream

	virtual BOOL	__stdcall Clear() = 0;						// erase all data

	virtual BOOL	__stdcall LoadPlayerData() = 0;				// sets fields to player's metadata store
	virtual BOOL	__stdcall LoadFullData() = 0;				// queries library plugins to get all available data

	virtual BOOL	__stdcall ApplyToPlayer(long flags) = 0;	// apply any changes to player's metadata store
	virtual BOOL	__stdcall ApplyToAll(long flags) = 0;		// apply any changes to player and library plugins

	virtual BOOL	__stdcall SetInfoIndex(long nIndex) = 0;	// 0 for album, 1-99 for tracks (for when type == cd)
	virtual long	__stdcall GetInfoIndex() = 0;				// current info index
	virtual long	__stdcall GetInfoIndexForMedia() = 0;		// info index for medianame

	virtual BOOL	__stdcall SetInfoByName(LPCWSTR pszName, LPCWSTR pValue, long flags) = 0;

	virtual BOOL	__stdcall GetInfoByName(LPCWSTR pszName, WCHAR* pValue, long* pnLength) = 0;
	virtual BOOL	__stdcall GetInfoAsLong(LPCWSTR pszName, long* pValue) = 0;
	virtual BOOL	__stdcall GetInfoByIndex(long index, WCHAR* pszName, long* pnNameLen, WCHAR* pValue, long* pnLength) = 0;
};

#endif //IQCDMEDIAINFO_H