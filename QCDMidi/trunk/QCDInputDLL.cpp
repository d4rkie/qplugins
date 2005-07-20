//-----------------------------------------------------------------------------
//
// File:	QCDInputDLL.cpp
//
// About:	See QCDInputDLL.h
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

// QCDMidi Input Plugin for QCD
// Created by Anthony Cozzolino
// Copyright 2003

// Things to do: implement a MidiException class instead of using errors
// Use output plugins (IDirectMusicPort8::GetFormat and Read)
// Make unicode
// Clean up code
// Add new extension capabilities to options dialog

//==Preprocessors==
#define INITGUID						// define required for DirectMusic
#pragma comment(lib, "winmm.lib")		// for timeGetTime()
#pragma comment(lib, "dsound.lib")		// for DirectMusic

#include "midi.h"						// for MIDI class
#include "QCDInputDLL.h"				// for QCD Functions
#include "resource.h"					// for resources
#include <string>						// for string type

using std::string;						// using the standard string class

//==Thread Flags Structure==
struct ThreadFlags
{
	string mediaName;					// filename of song
	int playFrom;						// start position
	int playTo;							// end position
};

//==Prototypes==
LRESULT CALLBACK AboutProc(HWND hwnd, UINT msg,
						   WPARAM wparam, LPARAM lparam);	// About Dialog Callback
DWORD WINAPI ThreadProc(ThreadFlags& threadFlags);			// Thread Function

//==Global Variables==
const char* PLUGINSTRING = "MIDI Input Plug-in v1.8";		// Plugin browser string
const char* EXTENSIONS = "MID:RMI:MIDI";					// Recognized extensions
MIDI midi;													// The playing midi file		
ThreadFlags tflags;											// Thread flags
HANDLE threadHandle = INVALID_HANDLE_VALUE;					// Handle to the main thread
HANDLE stopEvent = NULL;									// Stop midi event
HANDLE pauseEvent = NULL;									// Pause midi event
HANDLE volumeEvent = NULL;									// Volume change event
HANDLE readyEvent = NULL;									// Ready event

//==QCD Global Variables==
HINSTANCE		hInstance;									// Instance of the player
HWND			hwndPlayer;									// Handle to the player
QCDModInitIn	QCDCallbacks;								// Info structure

//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		hInstance = hInst;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------

PLUGIN_API QCDModInitIn* INPUTDLL_ENTRY_POINT(QCDModInitIn *ModInit, QCDModInfo *ModInfo)
{
	QCDCallbacks.size						= sizeof(QCDModInitIn);
	QCDCallbacks.version					= PLUGIN_API_VERSION;
	QCDCallbacks.toModule.Initialize		= Initialize;
	QCDCallbacks.toModule.ShutDown			= ShutDown;
	QCDCallbacks.toModule.GetTrackExtents	= GetTrackExtents;
	QCDCallbacks.toModule.GetMediaSupported	= GetMediaSupported;
	QCDCallbacks.toModule.Play				= Play;
	QCDCallbacks.toModule.Pause				= Pause;
	QCDCallbacks.toModule.Stop				= Stop;
	QCDCallbacks.toModule.About				= About;
	QCDCallbacks.toModule.Configure			= Configure;	
	QCDCallbacks.toModule.SetEQ				= NULL;
	QCDCallbacks.toModule.SetVolume			= SetVolume;

	return &QCDCallbacks;
}

BOOL Initialize(QCDModInfo *ModInfo, int flags)
{
	char inifile[MAX_PATH];

	// Set plugin string for plugin browser
	ModInfo->moduleString = (char*)PLUGINSTRING;

	hwndPlayer = (HWND)QCDCallbacks.Service(opGetParentWnd, 0, 0, 0);
	QCDCallbacks.Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);
	
	//
	// TODO : Register your extensions. 
	// ModInfo->moduleExtensions must point to non-volatile memory
	//
	// Ex: ModInfo->moduleExtensions = "WAV:MP3";
	//
	ModInfo->moduleExtensions = (char*)EXTENSIONS;

	//
	// TODO: all your plugin initialization here
	//
	
	// Fill ThreadFlags structure
	tflags.mediaName = "";
	tflags.playFrom = 0;
	tflags.playTo = 0;

	// Initialize thread handle
	threadHandle = INVALID_HANDLE_VALUE;

	// Create Stop Event
	stopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(stopEvent == INVALID_HANDLE_VALUE)
	{
		MessageBox(hwndPlayer, "Error creating \"Stop Event\"",
				   "QCDMidi Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Create Pause Event
	pauseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(pauseEvent == INVALID_HANDLE_VALUE)
	{
		MessageBox(hwndPlayer, "Error creating \"Pause Event\"",
				   "QCDMidi Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Create Volume Event
	volumeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(volumeEvent == INVALID_HANDLE_VALUE)
	{
		MessageBox(hwndPlayer, "Error creating \"Volume Event\"",
				   "QCDMidi Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Create Ready Event
	readyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(readyEvent == INVALID_HANDLE_VALUE)
	{
		MessageBox(hwndPlayer, "Error creating \"Ready Event\"",
				   "QCDMidi Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	//
	// TODO : Stop playback, cleanup
	//

	// Stop playing track
	Stop((tflags.mediaName).c_str(), 0);

	// Clean up memory
	CloseHandle(threadHandle);
	CloseHandle(stopEvent);
	CloseHandle(pauseEvent);
	CloseHandle(volumeEvent);
}

//-----------------------------------------------------------------------------

BOOL GetMediaSupported(LPCSTR medianame, MediaInfo *mediaInfo) 
{
	//
	// TODO : Verify that you support this media.
	//
	// Return FALSE to indicate this plugin does not support 'medianame'
	//
	// Return TRUE to indicate plugin is probably capable of supporting 
	// 'medianame'. I say probably, since there will another chance during
	// the Play call to determine this more accurately.
	//
	// If media is supported, populate applicable fields in MediaInfo
	// Note: mediaInfo may be NULL in some cases - check!


	// Make sure mediaInfo is not NULL
	if (!medianame || !*medianame)
	{
		return FALSE;
	}

	// Set track information if it is a recognized file
	char *ch = strrchr(medianame, '.');
	if (ch)
	{
		if (stricmp(ch, ".mid") == 0 || stricmp(ch, ".MID") == 0||
			stricmp(ch, ".rmi") == 0 || stricmp(ch, ".RMI") == 0||
			stricmp(ch, ".midi") == 0 || stricmp(ch, ".MIDI") == 0)
		{
			if (mediaInfo)
			{
				// fill in MediaInfo
				strcpy(mediaInfo->mediaFile, medianame);
				mediaInfo->mediaType = DIGITAL_FILE_MEDIA;
				mediaInfo->op_canSeek = TRUE;	
			}
			return TRUE;
		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------

BOOL GetTrackExtents(LPCSTR medianame, TrackExtents *ext, int flags)
{
	//
	// TODO : Fill the TrackExtents fields with appropriate information
	//		  about the track duration
	//
	// Return TRUE for success, FALSE for failure. 
	// A FALSE return will cause the track to not be loaded.
	// 
	
	// Fill Track Extents
	ext->track = 1;
	ext->start = 0;
	ext->unitpersec = 1000;
	ext->end = midi.GetLength(medianame) * 1000;
	HANDLE hMIDI = CreateFile(medianame, GENERIC_READ, FILE_SHARE_READ, NULL,
							 0, 0, NULL);
	ext->bytesize = GetFileSize(hMIDI, NULL);
	CloseHandle(hMIDI);

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL Play(LPCSTR medianame, int playfrom, int playto, int flags)
{
	//
	// TODO : Play medianame
	//
	// The parameters 'playfrom' and 'playto' set the extent of playback and
	// will be in the range set in GetTrackExtents above
	//
	// return TRUE for success (playback will start)
	// return FALSE for failure (playback will go to next track)
	//
	// return -1 for failure 
	//         (player will attempt to find alternate plugin to play medianame.
	//          use only if this plugin turns out not to be the one for playing
	//          medianame)

	// Confirm this is a midi file
	string file = medianame;
	if(file.substr(file.find_last_of('.') + 1, 3) != "mid" &&
       file.substr(file.find_last_of('.') + 1, 3) != "MID" &&
	   file.substr(file.find_last_of('.') + 1, 3) != "rmi" &&
	   file.substr(file.find_last_of('.') + 1, 3) != "RMI")
	{
		return -1;
	}

	// Stop all playing songs
	if(threadHandle != INVALID_HANDLE_VALUE)
	{
		ResetEvent(readyEvent);
		Stop((tflags.mediaName).c_str(), 0);
		WaitForSingleObject(readyEvent, INFINITE);
	}
	QCDCallbacks.toPlayer.OutputStop(STOPFLAG_FORCESTOP);

	// Fill ThreadFlags Structure
	tflags.mediaName = medianame;
	tflags.playFrom = playfrom;
	tflags.playTo = playto;
	
	// Start Initialization Thread
	CloseHandle(threadHandle);
	DWORD threadID;
	threadHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadProc,
								&tflags, 0, &threadID);

	if(threadHandle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	SetEvent(volumeEvent);

	// Update position
	QCDCallbacks.toPlayer.PositionUpdate(playfrom);
	
	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL Stop(LPCSTR medianame, int flags)
{
	//
	// TODO : Stop plugin playback
	//

	
	// return TRUE for success
	// return FALSE for failure

	// Tell midi thread to stop playing
	SetEvent(stopEvent);

	// Stop thread
	CloseHandle(threadHandle);
	threadHandle = INVALID_HANDLE_VALUE;

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL Pause(LPCSTR medianame, int flags)
{
	//
	// TODO : Pause playback, send back paused notification
	// flags = 1 - pause, 0 - unpause
	//

	/* example code - digital files usually pause output

	if (QCDCallbacks->toPlayer.OutputPause(flags))
	{
		// return TRUE for success
		return TRUE;
	}

	*/

	// Tell midi thread to pause playback
	SetEvent(pauseEvent);

	// return FALSE for failure
	return TRUE;
}

//-----------------------------------------------------------------------------

void SetVolume(int levelleft, int levelright, int flags)
{
	//
	// TODO : Update plugin volume settings
	//
	// Usually just call:
	// QCDCallbacks->toPlayer.OutputSetVol(levelleft, levelright, flags);
	//

	SetEvent(volumeEvent);
}

//-----------------------------------------------------------------------------

BOOL GetCurrentPosition(LPCSTR medianame, long *track, long *offset)
{
	//
	// TODO : Gets current playing track and offset - must be as exact
	// and as current as possible. Return TRUE for success.
	// set *track to current track number (always 1 for digital files or streams)
	// set *offset to current offset in track (same units as used in GetTrackExtents)
	//
	// For audio playing through output plugin, call:
	// return QCDCallbacks->toPlayer.OutputGetCurrentPosition((UINT*)offset, 0);
	//
		

	return FALSE;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	//
	// TODO : Show your "configuration" dialog.
	//
	MessageBox(hwndPlayer, "No configuration available.", "Midi Plugin for QCD", MB_OK);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	//
	// TODO : Show your "about" dialog.
	//
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwndPlayer,
		reinterpret_cast<DLGPROC>(AboutProc));
}

//-----------------------------------------------------------------------------

//==About Procedure Callback Function==
LRESULT CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
	// Command Message
	case WM_COMMAND:
		{
			switch (HIWORD(wparam))
			{
			// Button Clicked Message
			case BN_CLICKED:
				{
					switch(LOWORD(wparam))
					{
					// OK Button Clicked
					case IDC_OK:
						{
							// Close Dialog
							EndDialog(hwnd, FALSE);
							return TRUE;
						}
					}
					return TRUE;
				}
			}

			break;
		}
	}

	return FALSE;
}

//==Thread Function==
DWORD WINAPI ThreadProc(ThreadFlags& threadFlags)
{
	// Copy thread flags to a local variable
	ThreadFlags tf;
	tf.mediaName = threadFlags.mediaName;
	tf.playFrom = threadFlags.playFrom;
	tf.playTo = threadFlags.playTo;

	DWORD lastTime, thisTime;
	bool pause = false;
	char tempo[80];
	string statusMessage;

	// Load and play midi
	midi.InitializeMidi(hwndPlayer);
	midi.LoadMidi((tf.mediaName).c_str());
	midi.PlayMidi(tf.playFrom, tf.playTo, QCDCallbacks.Service(opGetTrackLength, NULL, -1, 1));

	/*WAVEFORMATEX waveFormat = midi.GetFormat();
	QCDCallbacks.toPlayer.OutputOpen(tf.mediaName.c_str(), &waveFormat);*/
	
	lastTime = timeGetTime();

	while(1)
	{
		thisTime = timeGetTime();

		// Check for stop event
		if(WaitForSingleObject(stopEvent, 0) == WAIT_OBJECT_0)
		{
			midi.CloseMidi();
			SetEvent(readyEvent);
			break;
		}

		// Check for pause event
		if(WaitForSingleObject(pauseEvent, 0) == WAIT_OBJECT_0)
		{
			midi.PauseMidi(pause);
			pause = !pause;
		}

		// Check for volume event
		if(WaitForSingleObject(volumeEvent, 0) == WAIT_OBJECT_0)
		{
			midi.SetMidiVolume((double)QCDCallbacks.Service(opGetVolume, NULL, 0, 0));
		}

		// Update position and tempo
		if(thisTime >= lastTime + 1000 && !pause)
		{
			double pos = midi.GetPosition(QCDCallbacks.Service(opGetTrackLength, NULL, -1, 1),
				                          tf.playFrom);
			if(pos == -1)
			{
				QCDCallbacks.toPlayer.PlayDone((tf.mediaName).c_str());
			}
			else
			{
				QCDCallbacks.toPlayer.PositionUpdate(pos);
				lastTime = thisTime;
				AudioInfo info;
				info.struct_size = sizeof(AudioInfo);
				info.frequency = 0;
				info.bitrate = 0;
				info.mode = 0;
				char text[9];
				char temp[5];
				lstrcpy(text, itoa(midi.GetTempo(), temp, 10));
				lstrcat(text, "bpm");
				lstrcpyn(info.text, text, sizeof(info.text));
				QCDCallbacks.Service(opSetAudioInfo, &info, sizeof(AudioInfo), 0);
			}
		}

		/*DWORD usedBytes;
		BYTE* buffer = midi.GetData(usedBytes);
		WriteDataStruct wd;
		wd.bytelen = usedBytes;
		wd.data = buffer;
		wd.markerend = 0;
		wd.markerstart = 0;
		wd.bps = waveFormat.wBitsPerSample;
		wd.nch = waveFormat.nChannels;
		wd.numsamples = usedBytes / (wd.bps / 8) / waveFormat.nChannels;
		wd.srate = waveFormat.nSamplesPerSec;
		QCDCallbacks.toPlayer.OutputWrite(&wd);*/

		Sleep(50);	// ease up on CPU usage
	}

	// Exit the thread
	ExitThread(0);
}