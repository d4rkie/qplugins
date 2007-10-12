//-----------------------------------------------------------------------------
// 
// File:	QMPPlaylists.h
//
// About:	QCD Player Playlist module DLL interface.  For more documentation, see
//			QCDModPlaylist.h.
//
// Authors:	Written by Paul Quinn and Richard Carlson.
//
//	QCD multimedia player application Software Development Kit Release 1.0.
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

#ifndef QMPPlaylists_H
#define QMPPlaylists_H

#include "QCDModPlaylist.h"

#include "QMPModule.h"


//////////////////////////////////////////////////////////////////////////

class QMPPlaylists : public QMPModule< QMPPlaylists, QCDModInitPL >
{
	friend class QMPModule< QMPPlaylists, QCDModInitPL >;
protected:
	QMPPlaylists(void)
	{
		QCDCallbacks.version				= PLUGIN_API_VERSION_UNICODE;

		QCDCallbacks.toModule.Initialize	= Initialize;
		QCDCallbacks.toModule.ShutDown		= ShutDown;
		QCDCallbacks.toModule.InitPlaylist	= InitPlaylist;
		QCDCallbacks.toModule.LoadEntries	= LoadEntries;
		QCDCallbacks.toModule.SaveEntry		= SaveEntry;
		QCDCallbacks.toModule.DeinitPlaylist= DeinitPlaylist;
		QCDCallbacks.toModule.Configure		= NULL; //Configure;
		QCDCallbacks.toModule.About			= About;
	}

private:
	static BOOL Initialize(QCDModInfo *modInfo, int flags);
	static void ShutDown(int flags);

	static void Configure(int flags);
	static void About(int flags);

	static void* InitPlaylist(const char* playlist, int inflags, int *outflags);
	static int	 LoadEntries(void* plHandle, void* plData, int flags);
	static int	 SaveEntry(void* plInstance, const char* medianame, PLEntryData* plEntryData, int flags);
	static void  DeinitPlaylist(void* plInstance, int flags);
};

#endif //QMPPlaylists_H

