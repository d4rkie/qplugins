//-----------------------------------------------------------------------------
//
// File:	QCDModInput.h
//
// About:	Input plugin module interface.  This file is published with the 
//			Input plugin SDK.
//
// Authors:	Written by Paul Quinn
//
// Copyright:
//
//	QCD multimedia player application Software Development Kit Release 1.0.
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

#ifndef QCDMODINPUT_H
#define QCDMODINPUT_H

#include "QCDModDefs.h"
#include "IQCDMediaDecoder.h"

// name of the DLL export for input plugins
#define INPUTDLL_ENTRY_POINT		PLUGIN_ENTRY_POINT(InputModule2)

// TestFormat return values
#define TESTFORMAT_ACCEPTED			0
#define TESTFORMAT_UNACCEPTED		-1
#define TESTFORMAT_FAILURE			-2
#define TESTFORMAT_NOTIMPL			-3

// stop will receive one of these flags (pass to output plugin's stop())
#define STOPFLAG_FORCESTOP			0		// stop occuring due to user action (player will enter stopped state)
#define STOPFLAG_PLAYDONE			1		// stop occuring due to current track completion
#define STOPFLAG_SHUTDOWN			2		// stop occuring due to player shutdown
#define STOPFLAG_PLAYSKIP			3		// stop occuring due to current track skip
#define STOPFLAG_ALLDONE			4		// stop occuring due to playlist completion (player will enter stopped state)

// play flags (player sends these to play function)
#define PLAYFLAG_PLAYBACK			0x0
#define PLAYFLAG_ENCODING			0x1
#define PLAYFLAG_SEEKING			0x2

// pause flags (player sends these to pause function, plugin sends to PlayPaused function)
#define PAUSE_DISABLED				0		// Pause() call is to unpause playback
#define PAUSE_ENABLED				1		// Pause() call is to pause playback

// eject flags (player sends these to eject function)
#define EJECT_TOGGLE				0		// toggle current open/close state of media tray
#define EJECT_ENSURE_OPEN			1		// set state of media tray to open
#define EJECT_ENSURE_CLOSED			2		// set state of media tray to closed
									
// play function return values (play function should return one of these)
#define PLAYSTATUS_SUCCESS			1		// play call succeeded
#define PLAYSTATUS_FAILED			0		// play call failed (player will go to next track)
#define PLAYSTATUS_UNSUPPORTED		-1		// file is not supported by this plugin afterall (if guess made by GetMediaSupported was wrong). 
                                        	//     Player will find alternate plugin to play track, or go to next track
#define PLAYSTATUS_STOPPLAYBACK		-2		// play call did not succeed, and player should not go to next track

// Wave Marker flags
#define WAVE_VIS_DATA_ONLY			-1		// set to WaveDataStruct.markerstart in OutputWrite() call have data only go to vis 
											// and not to output plugin
// GetTrackExtents flags
#define GETTRACKEXTENTS_DEFAULT		0x0		  // default flag, track extents need to be determined
#define GETTRACKEXTENTS_VERIFY		0x100   // when passed to GetTrackExtents, indicates TrackExtents parameter has prepopulated values
                                            // that the plug-in can either ignore, verify, and/or modify to its needs

//-----------------------------------------------------------------------------
// Input Module
//-----------------------------------------------------------------------------
typedef struct _QCDModInitIn
{
	unsigned int		size;			// size of init structure
	unsigned int		version;		// plugin structure version (set to PLUGIN_API_VERSION)
	PluginServiceFunc	Service;		// player supplied services callback

	struct _toPlayer
	{
		void (*PositionUpdate)(unsigned int position);
		void (*PlayStopped)(const char* medianame, int flags);		// notify player of play stop
		void (*PlayStarted)(const char* medianame);					// notify player of play start
		void (*PlayPaused)(const char* medianame, int flags);		// notify player of play pause
		void (*PlayDone)(const char* medianame, int flags);			// notify player when play done
		void (*PlayTrackChanged)(const char* medianame);			// notify player when playing track changes (cd audio relevant only)
		void (*MediaEjected)(const char* medianame);				// notify player of media eject (cd audio relevant)
		void (*MediaInserted)(const char* medianame, int flags);	// notify player of media insert (cd audio relevant)

																	// output plugin calls
		int  (*OutputOpen)(const char* medianame, WAVEFORMATEX* wf);// open output for wave data
		int  (*OutputWrite)(WriteDataStruct*);						// send PCM audio data to output 
																		// (blocks until write completes, thus if output is paused can 
																		// block until unpaused)
		int  (*OutputDrain)(int flags);								// wait for all output to complete (blocking)
		int  (*OutputDrainCancel)(int flags);						// break a drain in progress
		int  (*OutputFlush)(unsigned int marker);						// flush output upto marker
		int  (*OutputStop)(int flags);								// stop output
		int  (*OutputPause)(int flags);								// pause output

		int  (*OutputSetVol)(int levelleft, int levelright, int flags);
		int  (*OutputGetCurrentPosition)(unsigned int *position, int flags);

		int  (*OutputTestFormat)(WAVEFORMATEX* wf, int flags);		// test to see if waveformat is acceptable to output
																		// will fail silently, return value will be one of TESTFORMAT_*
		void *Reserved[9];
	} toPlayer;

	struct _toModule
	{
		int  (*Initialize)(QCDModInfo *modInfo, int flags);			// initialize plugin
		void (*ShutDown)(int flags);								// shutdown plugin

		int  (*Play)(const char* medianame, int playfrom, int playto, int flags);	// start playing playfrom->playto
		int  (*Stop)(const char* medianame, int flags);				// stop playing
		int  (*Pause)(const char* medianame, int flags);			// pause playback
		int  (*Eject)(const char* medianame, int flags);			// eject media
		void (*SetEQ)(EQInfo*);										// update EQ settings

		int  (*GetMediaSupported)(const char* medianame, MediaInfo *mediaInfo);			// does plugin support medianame (and provides info for media)
		int  (*GetTrackExtents)(const char* medianame, TrackExtents *ext, int flags);	// get media start, end & units
		int  (*GetCurrentPosition)(const char* medianame, long *track, long *offset);	// get playing media's position

		void (*Configure)(int flags);									// launch configuration
		void (*About)(int flags);										// launch about info

		void (*SetVolume)(int levelleft, int levelright, int flags);	// level 0 - 100

		IQCDMediaDecoder* (*CreateDecoderInstance)(const WCHAR* medianame, int flags); // create independant decoder instance to facilitate IQCDMediaDecoder

		void *Reserved[9];
	} toModule;

} QCDModInitIn;

#endif //QCDMODINPUT_H
