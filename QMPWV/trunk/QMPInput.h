//-----------------------------------------------------------------------------
// 
// File:	QCDInputDLL.h
//
// About:	QCD Player Input module DLL interface.  For more documentation, see
//			QCDModInput.h.
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

#ifndef QMPInput_H
#define QMPInput_H

#include "QCDModInput.h"

#include "QMPModule.h"


//////////////////////////////////////////////////////////////////////////

class QMPInput : public QMPModule< QMPInput, QCDModInitIn >
{
	friend class QMPModule< QMPInput, QCDModInitIn >;
protected:
	QMPInput(void)
	{
		QCDCallbacks.version					= PLUGIN_API_VERSION_UNICODE;
		QCDCallbacks.toModule.Initialize		= Initialize;
		QCDCallbacks.toModule.ShutDown			= ShutDown;
		QCDCallbacks.toModule.GetTrackExtents	= GetTrackExtents;
		QCDCallbacks.toModule.GetMediaSupported	= GetMediaSupported;
		QCDCallbacks.toModule.Play				= Play;
		QCDCallbacks.toModule.Pause				= Pause;
		QCDCallbacks.toModule.Stop				= Stop;
		QCDCallbacks.toModule.About				= About;
		QCDCallbacks.toModule.Configure			= Configure;	
		QCDCallbacks.toModule.SetEQ				= NULL;//SetEQ;
		QCDCallbacks.toModule.SetVolume			= SetVolume;

		// IQCDMediaDecoder
		QCDCallbacks.toModule.CreateDecoderInstance = CreateDecoderInstance;
	}

private:
	// Calls from the Player
	static int  GetMediaSupported(const char* medianame, MediaInfo *mediaInfo);
	static int  GetTrackExtents(const char* medianame, TrackExtents *ext, int flags);
	static int  GetCurrentPosition(const char* medianame, long *track, long *offset);

	static void SetEQ(EQInfo*);
	static void SetVolume(int levelleft, int levelright, int flags);

	static int  Play(const char* medianame, int framefrom, int frameto, int flags);
	static int  Pause(const char* medianame, int flags);
	static int  Stop(const char* medianame, int flags);
	static int  Eject(const char* medianame, int flags);

	static int  Initialize(QCDModInfo *ModInfo, int flags);
	static void ShutDown(int flags);
	static void Configure(int flags);
	static void About(int flags);

	static IQCDMediaDecoder* CreateDecoderInstance(const WCHAR* medianame, int flags);

private:
	static HWND hwndPlayer;
	static DWORD WINAPI DecodeThread(LPVOID lpParameter);
};

#endif //QMPInput_H

