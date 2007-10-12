//-----------------------------------------------------------------------------
//
// File:	QCDModPlaylist
//
// About:	Playlist plugin module interface.  This file is published with the 
//			Playlist plugin SDK.
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

#ifndef QCDMODPLAYLIST_H
#define QCDMODPLAYLIST_H

#include "QCDModDefs.h"

// name of the DLL export for output plugins
#define PLAYLISTDLL_ENTRY_POINT		PLUGIN_ENTRY_POINT(PlaylistModule)

// playlist init flags
#define PLAYLIST_INIT_READ			0x1			// playlist will be read from
#define PLAYLIST_INIT_WRITE			0x2			// playlist will be written to
#define PLAYLIST_INIT_APPEND		0x4			// playlist writes should be appended
									
// playlist saving flags

// On InitPlaylist, set outflags to one or more of these 
// flags to receive the respective data. 
#define PLAYLIST_WANT_ARTIST		0x100		// want artist name
#define PLAYLIST_WANT_ALBUM			0x200		// want album name
#define PLAYLIST_WANT_TITLE			0x400		// want track title
#define PLAYLIST_WANT_DISPLAYTITLE	0x800		// want entry name as appears in player's playlist
#define PLAYLIST_WANT_TRACKEXTENTS	0x1000		// want track duration information
#define PLAYLIST_WANT_ALL			(PLAYLIST_WANT_ARTIST|PLAYLIST_WANT_ALBUM|PLAYLIST_WANT_TITLE|PLAYLIST_WANT_TRACKEXTENTS)

// Write/Append Playlist flags
#define PLAYLIST_WRITE_NOTIFY		0x100		// notify all library plugins about new/updated playlist
#define PLAYLIST_WRITE_UNIXPATHS	0x200		// use '/' path delimeters instead of '\' when writing playlists

//-----------------------------------------------------------------------------

struct PLEntryData
{
	int					struct_size;
	
	enum MediaTypes		mediaType;

	int					extentsValid;
	struct TrackExtents	trackExtents;

	unsigned long		cdid;

	char*				title;
	char*				album;
	char*				artist;
	char*				genre;
	char*				year;
	void*				reserved[10];
};

//-----------------------------------------------------------------------------

struct QCDModInitPL
{
	UINT				size;			// size of init structure
	UINT				version;		// plugin structure version (set to PLUGIN_API_VERSION)
	PluginServiceFunc	Service;		// player supplied services callback

	struct
	{
		int  (*LoadEntry)(void* plHandle, const char* medianame, struct PLEntryData* plEntryData, int flags);
		int  (*LoadComplete)(void* plHandle, int flags);

		void *Reserved[6];
	} toPlayer;

	struct 
	{
		void*  (*InitPlaylist)(const char* playlist, int inflags, int *outflags);
		int	   (*LoadEntries)(void* plHandle, void* plData, int flags);
		int	   (*SaveEntry)(void* plData, const char* medianame, struct PLEntryData* plEntryData, int flags);
		void   (*DeinitPlaylist)(void* plData, int flags);

		int	   (*Initialize)(struct QCDModInfo *modInfo, int flags);
		void   (*ShutDown)(int flags);
		void   (*Configure)(int flags);
		void   (*About)(int flags);

		void *Reserved[4];
	} toModule;
};


#endif //QCDMODPLAYLIST_H