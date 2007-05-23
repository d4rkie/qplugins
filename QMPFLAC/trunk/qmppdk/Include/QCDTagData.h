//-----------------------------------------------------------------------------
//
// File:	QCDTagData
//
// About:	Data structures for IQCDTagInfo and QCDModTagEditor2 interfaces
//
// Authors:	Written by Paul Quinn
//
// Copyright:
//
//	QCD multimedia player application Software Development Kit
//
//	Copyright (C) 1997-2005 Quinnware
//
//	This code is free.  If you redistribute it in any form, leave this notice
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

#ifndef QCDTAGDATA_H
#define QCDTAGDATA_H

//-----------------------------------------------------------------------------

// Defined tag fields

	// common
#define		QCDTag_Title					L"TITLE"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Artist					L"ARTIST"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_AlbumArtist				L"ALBUMARTIST"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Album					L"ALBUM"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Genre					L"GENRE"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Year						L"YEAR"				// QTD_TYPE_STRINGUNICODE
#define		QCDTag_TrackNumber				L"TRACKNUMBER"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Lyricist					L"LYRICIST"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Mood						L"MOOD"				// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Remix					L"REMIX"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Comment					L"COMMENT"			// QTAGDATA_HEADER_COMMENT* (QTD_TYPE_BINARY)
#define		QCDTag_Artwork					L"ARTWORK"			// QTAGDATA_HEADER_ARTWORK* (QTD_TYPE_BINARY)
#define		QCDTag_Lyrics					L"LYRICS"			// TBD
											
	// classical							
#define		QCDTag_Composer					L"COMPOSER"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Conductor				L"CONDUCTOR"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Orchestra				L"ORCHESTRA"		// QTD_TYPE_STRINGUNICODE

	// originals
#define		QCDTag_OrigArtist				L"ORIGARTIST"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_OrigRelYear				L"ORIGRELYEAR"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_OrigTitle				L"ORIGTITLE"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_OrigFilename				L"ORIGFILENAME"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_OrigLyricsist			L"ORIGLYRICSIST"	// QTD_TYPE_STRINGUNICODE

	// misc						
#define		QCDTag_YearReleased				L"YEARREL"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_SubTitle					L"SUBTITLE"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_DiscNumber				L"DISCNUMBER"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Rating					L"RATING"			// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Copyright				L"COPYRIGHT"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_Publisher				L"PUBLISHER"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_EncodedBy				L"ENCODEDBY"		// QTD_TYPE_STRINGUNICODE
#define		QCDTag_GracenoteFileId			L"GRACENOTEFILEID"	// QTD_TYPE_BINARY
#define		QCDTag_GracenoteExtData			L"GRACENOTEEXTDATA"	// QTD_TYPE_BINARY
#define		QCDTag_BPM						L"BPM"				// QTD_TYPE_STRINGUNICODE

	// replaygain
#define		QCDTag_ReplayGain_Album_Gain	L"REPLAYGAIN_ALBUM_GAIN" // QTD_TYPE_STRINGUNICODE
#define		QCDTag_ReplayGain_Track_Gain	L"REPLAYGAIN_TRACK_GAIN" // QTD_TYPE_STRINGUNICODE
#define		QCDTag_ReplayGain_Album_Peak	L"REPLAYGAIN_ALBUM_PEAK" // QTD_TYPE_STRINGUNICODE
#define		QCDTag_ReplayGain_Track_Peak	L"REPLAYGAIN_TRACK_PEAK" // QTD_TYPE_STRINGUNICODE

//-----------------------------------------------------------------------------
// QTAGDATA_TYPE
// All tag fields will be declared on of the following types
// Note: due to fields be set by any plugin, do not anticipate proper
//       type setting or formatting of tag data.

enum QTAGDATA_TYPE
{
	QTD_TYPE_UNKNOWN		= 0,
	QTD_TYPE_LONG			= 1,
	QTD_TYPE_BINARY			= 2,
	QTD_TYPE_STRINGUNICODE	= 3
};

//-----------------------------------------------------------------------------
// Struct definitions for complex tag data type (noted in tag field list above)
// 

// these structure are byte aligned
#include <pshpack1.h>

struct QTAGDATA_HEADER_ARTWORK
{
	LPWSTR	pszMimeType;
	BYTE	bPictureType;
	LPWSTR	pwszDescription;
	DWORD	dwDataLen;
	BYTE*	pbData;
};


struct QTAGDATA_HEADER_COMMENT
{
	BYTE	bLang[3];
	LPWSTR	pwszDescription;
	LPWSTR	pwszComment;
};


struct QTAGDATA_HEADER_LYRICS
{
	BYTE	bLang[3];
	LPWSTR	pwszDescription;
	LPWSTR	pwszLyrics;
};

#include <poppack.h>

//-----------------------------------------------------------------------------
// Legacy type names
//

#define QCD_TAGDATA_TYPE			QTAGDATA_TYPE;
#define QTD_STRUCT_ARTWORK			QTAGDATA_HEADER_ARTWORK;
#define QTD_STRUCT_COMMENT			QTAGDATA_HEADER_COMMENT;
#define QTD_STRUCT_LYRICS			QTAGDATA_HEADER_LYRICS;


#endif // QCDTAGADATA_H