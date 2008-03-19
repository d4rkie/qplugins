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

#define     QCDInfo_Stream_StationURL       L"StationURL"
#define     QCDInfo_Stream_ContentType      L"ContentType"
#define     QCDInfo_Stream_FileFormat       L"FileFormat"
#define     QCDInfo_Stream_StationName      QCDInfo_ArtistAlbum
#define     QCDInfo_Stream_LastPlaying      QCDInfo_TitleTrack
#define     QCDInfo_Stream_Genre            QCDInfo_GenreTrack
#define     QCDInfo_Stream_Comment          QCDInfo_Comment
#define     QCDInfo_Stream_Bitrate          QCDInfo_Bitrate

#define		QCDInfo_ReplayGain_Album_Gain	L"ReplayGain_Album_Gain"
#define		QCDInfo_ReplayGain_Track_Gain	L"ReplayGain_Track_Gain"
#define		QCDInfo_ReplayGain_Album_Peak	L"ReplayGain_Album_Peak"
#define		QCDInfo_ReplayGain_Track_Peak	L"ReplayGain_Track_Peak"

// defined flags for SetInfoByName

#define MEDIAINFO_ALWAYSSET			0x0
#define MEDIAINFO_SETIFEMPTY		0x10
#define MEDIAINFO_SETIFNOTEMPTY		0x20


// <title IQCDMediaInfo>
// 
// Provides container to request and store metadata about a single piece of media.  


struct _declspec(novtable)
IQCDMediaInfo
{
	// Increment reference count for interface.
	virtual ULONG	__stdcall AddRef();
	// Decrement reference count on interface.
	virtual ULONG	__stdcall Release();

	// Gets name of media this interface is for
	virtual LPCWSTR __stdcall GetMediaName();
	// Get type of media this interface is for 
	virtual long	__stdcall GetMediaType();

	// Erase all metadata in interface
	virtual BOOL	__stdcall Clear();

	// Loads metadata stored in the player for set media
	virtual BOOL	__stdcall LoadPlayerData();
	// Loads metadata for the media from supporting library modules
	virtual BOOL	__stdcall LoadFullData();

	// Apply any metadata set in interface to player only (not to library plug-ins)
	virtual BOOL	__stdcall ApplyToPlayer(long flags);
	
	// Apply any metadata set in interface to player and library plug-ins
	virtual BOOL	__stdcall ApplyToAll(long flags);		

	// Set the info index members to read or write the correct information for the
	// media.
	virtual BOOL	__stdcall SetInfoIndex(long nIndex);
	// Gets the current info index
	virtual long	__stdcall GetInfoIndex();
	// Get the info index for medianame
	virtual long	__stdcall GetInfoIndexForMedia();

	// Sets the given value for the given field
	virtual BOOL	__stdcall SetInfoByName(LPCWSTR pszName, LPCWSTR pValue, long flags);

	// Retrieves the string value for a given field name
	virtual BOOL	__stdcall GetInfoByName(LPCWSTR pszName, WCHAR* pValue, long* pnLength);
	// Retrieves the integer value for given field name
	virtual BOOL	__stdcall GetInfoAsLong(LPCWSTR pszName, long* pValue);
	// Retrieves the string value for given index
	virtual BOOL	__stdcall GetInfoByIndex(long index, WCHAR* pszName, long* pnNameLen, WCHAR* pValue, long* pnLength);
};

#endif //IQCDMEDIAINFO_H