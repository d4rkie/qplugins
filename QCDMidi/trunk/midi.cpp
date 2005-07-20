//==Preprocessors==
#include "midi.h"
#include <windows.h>
#include <limits.h>
#include <dmusicf.h>

//==Default Constructor==
MIDI::MIDI():myLoaderObject(NULL),
			 myPerformanceObject(NULL),
			 mySegmentObject(NULL),
			 myDirectMusicObject(NULL),
			 myAudioPath(NULL),
			 myHandle(NULL),
			 myPauseTime(0),
			 myLength(0),
			 myLengthMS(0)
{
	// no extra initialization
}

//==Default Destructor==
MIDI::~MIDI()
{
	// no extra clean up
}

//==Initialize MIDI Method==
bool MIDI::InitializeMidi(HWND hwnd)
{
	// Initialize COM and IDirectMusic Classes
	CoInitialize(NULL);
    
    CoCreateInstance(CLSID_DirectMusicLoader, NULL, 
                     CLSCTX_INPROC, IID_IDirectMusicLoader8,
                     (void**)&myLoaderObject);


    CoCreateInstance(CLSID_DirectMusicPerformance, NULL,
                     CLSCTX_INPROC, IID_IDirectMusicPerformance8,
                     (void**)&myPerformanceObject);

	CoCreateInstance(CLSID_DirectMusicSegment, NULL,
					 CLSCTX_INPROC, IID_IDirectMusicSegment8,
					 (void**)&mySegmentObject);

	// Set window handle
	myHandle = hwnd;

	// Initialize DirectMusic
	HRESULT result;
	result = myPerformanceObject->InitAudio((IDirectMusic**)&myDirectMusicObject, NULL, 
											hwnd,              
											DMUS_APATH_SHARED_STEREOPLUSREVERB,  
											128, DMUS_AUDIOF_ALL,       
											NULL);
	if (result != S_OK)
	{
		MessageBox(myHandle, "Error initializing DirectSound.", "QCDMidi Error",
			       MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

//==Load MIDI Method==
bool MIDI::LoadMidi(const char* medianame)
{
	// Convert filename to UNICODE
	MultiByteToWideChar(CP_UTF8, 0, medianame, -1, myMediaName, MAX_PATH);
	
	// Load file
	if (FAILED(myLoaderObject->LoadObjectFromFile(
		       CLSID_DirectMusicSegment, IID_IDirectMusicSegment8,
               myMediaName, (LPVOID*) &mySegmentObject)))
    {
        MessageBox(myHandle, "Media failed to load.", "QCDMidi Error", MB_OK | MB_ICONERROR);
		CloseMidi();
        return false;
    }

	return true;
}

//==Tease Play MIDI Method==
bool MIDI::TeasePlayMidi(MUSIC_TIME& length, double& tempo)
{
	// Set DirectMusic to play all types of MIDI files
	mySegmentObject->SetParam(GUID_StandardMIDIFile,
							  0xFFFFFFFF, 0, 0, NULL);

	// Download Segment
	if(mySegmentObject->Download(myPerformanceObject) != S_OK)
	{
		MessageBox(myHandle, "Error trying to play.", "QCDMidi Error",
			       MB_OK | MB_ICONERROR);
		return false;
	}

	// Set starting point of the midi
	if(mySegmentObject->SetStartPoint(0) != S_OK)
	{
		MessageBox(myHandle, "Error setting start point.", "QCDMidiError", MB_OK | MB_ICONERROR);
		return false;
	}

	// Play 
	HRESULT result;
	IDirectMusicSegmentState* segState;

	result = myPerformanceObject->PlaySegmentEx(mySegmentObject, NULL, NULL, 0, 0, 
												&segState, NULL, NULL);  
    
	if (result != S_OK)
	{
		MessageBox(myHandle, "Error trying to play.", "QCDMidi Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// Get a handle to the segment state object
	segState->QueryInterface(IID_IDirectMusicSegmentState8,
							 (LPVOID*) &mySegmentStateObject);

	mySegmentObject->GetLength(&length);
	tempo = GetTempo();

	return true;

}

//==Play MIDI Method==
bool MIDI::PlayMidi(int playfrom, int playto, int trackLength, bool unpause)
{
	// Set DirectMusic to play all MIDI files
	mySegmentObject->SetParam(GUID_StandardMIDIFile, 
                              0xFFFFFFFF, 0, 0, NULL);

	// Download Segment
	if(mySegmentObject->Download(myPerformanceObject) != S_OK)
	{
		MessageBox(myHandle, "Error trying to play.", "QCDMidi Error",
			       MB_OK | MB_ICONERROR);
		return false;
	}

	// Create a standard audio path
	/*if(FAILED(myPerformanceObject->CreateStandardAudioPath(DMUS_APATH_STEREO, 64,
														   FALSE, &myAudioPath)))
	{
		MessageBox(myHandle, "Error trying to play.", "QCDMidi Error",
				   MB_OK | MB_ICONERROR);
		return false;
	}*/

	// Find start time of midi
	MUSIC_TIME startTime;
	if(unpause)
	{
		startTime = playfrom;
	}
	else
	{
		myLengthMS = trackLength;
		double percentage = (double)playfrom / (double)trackLength;
		MUSIC_TIME length;
		mySegmentObject->GetLength(&length);
		startTime = (double)length * percentage;
	}

	// Set starting point of the midi
	if(mySegmentObject->SetStartPoint(startTime) != S_OK)
	{
		MessageBox(myHandle, "Error setting start point.", "QCDMidiError", MB_OK | MB_ICONERROR);
		return false;
	}
	
	// Play 
	HRESULT result;
	IDirectMusicSegmentState* segState;

	result = myPerformanceObject->PlaySegmentEx(mySegmentObject, NULL, NULL, 0, 0, 
												&segState, NULL, NULL);  
    
	if (result != S_OK)
	{
		MessageBox(myHandle, "Error trying to play.", "QCDMidi Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// Get a handle to the segment state object
	segState->QueryInterface(IID_IDirectMusicSegmentState8,
							 (LPVOID*) &mySegmentStateObject);

	return true;
}

//==Stop MIDI Method==
bool MIDI::StopMidi()
{
	// Stop playing
	if (myPerformanceObject->IsPlaying(mySegmentObject, NULL) == S_OK)
	{
		HRESULT result;
		result = myPerformanceObject->StopEx(mySegmentObject, 0, 0);

		if (result != S_OK)
		{
			MessageBox(myHandle, "Error trying to stop.", "QCDMidi Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}
	
	return true;
}

//==Pause MIDI Method==
bool MIDI::PauseMidi(bool unpause)
{
	if(unpause && myPerformanceObject->IsPlaying(mySegmentObject, NULL) != S_OK)
	{
		// Unpause playback
		if(!PlayMidi(myPauseTime, myLengthMS, myLengthMS, true))
		{
			return false;
		}
	}
	else if(myPerformanceObject->IsPlaying(mySegmentObject, NULL) == S_OK)
	{
		// Get track information
		MUSIC_TIME curTime, startTime, startPoint, loopStart, loopEnd, length;
		DWORD repeats;

		if(myPerformanceObject->GetTime(NULL, &curTime) != S_OK)
		{
			MessageBox(myHandle, "Error pausing.", "QCDMidi Error", MB_OK | MB_ICONERROR);
			return false;
		}

		if(mySegmentStateObject->GetStartTime(&startTime) != S_OK)
		{
			MessageBox(myHandle, "Error pausing.", "QCDMidi Error", MB_OK | MB_ICONERROR);
			return false;
		}

		if(mySegmentStateObject->GetStartPoint(&startPoint) != S_OK)
		{
			MessageBox(myHandle, "Error pausing.", "QCDMidi Error", MB_OK | MB_ICONERROR);
			return false;
		}

		if(mySegmentObject->GetLoopPoints(&loopStart, &loopEnd) != S_OK)
		{
			MessageBox(myHandle, "Error pausing.", "QCDMidi Error", MB_OK | MB_ICONERROR);
			return false;
		}

		if(mySegmentObject->GetLength(&length) != S_OK)
		{
			MessageBox(myHandle, "Error pausing.", "QCDMidi Error", MB_OK | MB_ICONERROR);
			return false;
		}

		if(mySegmentObject->GetRepeats(&repeats) != S_OK)
		{
			MessageBox(myHandle, "Error pausing.", "QCDMidi Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// Save time offset
		myPauseTime = GetTimeOffset(curTime, startTime, startPoint, loopStart, loopEnd,
									length, repeats);

		// Stop playback
		if(myPerformanceObject->StopEx(mySegmentObject, 0, 0) != S_OK)
		{
			MessageBox(myHandle, "Error pausing.", "QCDMidi Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	return true;
}

//==Close MIDI Method==
void MIDI::CloseMidi()
{
	// Stop playback
	StopMidi();

	// Clean up objects
	if(myPerformanceObject)
	{
		myPerformanceObject->CloseDown();
		myPerformanceObject->Release();
	}
 
	if(myLoaderObject)
		myLoaderObject->Release(); 

    if(mySegmentObject)
		mySegmentObject->Release();

	if(myAudioPath)
		myAudioPath->Release();

	if(myDirectMusicObject)
		myDirectMusicObject->Release();
 
	myLoaderObject = NULL;
	myPerformanceObject = NULL;
	mySegmentObject = NULL;
	myDirectMusicObject = NULL;
	myAudioPath = NULL;

	// Uninitialize COM
    CoUninitialize();
}

//==Get Length Method==
unsigned int MIDI::GetLength(LPCSTR medianame)
{
	MUSIC_TIME length;
	double tempo;
	static MIDI midi;
	midi.InitializeMidi(NULL);
	midi.LoadMidi(medianame);
	midi.TeasePlayMidi(length, tempo);
	midi.StopMidi();
	midi.CloseMidi();
	return ((length / 768) * 60) / tempo;
	
	/*MCI_OPEN_PARMS	mci_open_params; // Holds midi information
	
	// Store midi information
	mci_open_params.lpstrDeviceType = "sequencer";
	mci_open_params.lpstrElementName = medianame; 
	
	// Opens Device
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mci_open_params);
	MCIDEVICEID DeviceHandle = mci_open_params.wDeviceID;

	

	MCI_SEEK_PARMS seek_params; // This structure is necessary for passing into the 
								// mciSendCommand() when doing a "seek"
	
	// Make sure midi is at beginning
	mciSendCommand(DeviceHandle,MCI_SEEK, MCI_SEEK_TO_START, (DWORD)&seek_params);

	

	MCI_SET_PARMS mci_set_params;
	mci_set_params.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	mciSendCommand(DeviceHandle, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)&mci_set_params);

	MCI_STATUS_PARMS mci_status_params;
	mci_status_params.dwItem = MCI_STATUS_LENGTH;
	
	mciSendCommand(DeviceHandle, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mci_status_params);
	
	unsigned int length = mci_status_params.dwReturn;

	MCI_GENERIC_PARMS close_params; // A generic parameter to pass into mciSendCommand()

	// Closes Device
	mciSendCommand(DeviceHandle,MCI_CLOSE,MCI_WAIT,(DWORD)&close_params);
	
	myLength = length;
	return (unsigned int)length;*/
}

//==Get Offset Method==
double MIDI::GetPosition(int trackLength, int playFrom)
{
	// Get track information
	MUSIC_TIME curTime, start, startTime, startPoint, length, loopStart, loopEnd;
	DWORD repeats;
	double percentage;

	if(myPerformanceObject->GetTime(NULL, &curTime) != S_OK)
	{
		MessageBox(myHandle, "Error getting position.", "QCDMidi Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	if(mySegmentStateObject->GetStartTime(&startTime) != S_OK)
	{
		MessageBox(myHandle, "Error getting position.", "QCDMidi Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	if(mySegmentObject->GetStartPoint(&startPoint) != S_OK)
	{
		MessageBox(myHandle, "Error getting position.", "QCDMidi Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	start = startTime + startPoint;

	if(mySegmentObject->GetLength(&length) != S_OK)
	{
		MessageBox(myHandle, "Error getting position.", "QCDMidi Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	if(mySegmentObject->GetLoopPoints(&loopStart, &loopEnd) != S_OK)
	{
		MessageBox(myHandle, "Error getting position.", "QCDMidi Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if(mySegmentObject->GetRepeats(&repeats) != S_OK)
	{
		MessageBox(myHandle, "Error getting position.", "QCDMidi Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// Get current time offset
	MUSIC_TIME offset = GetTimeOffset(curTime, startTime, startPoint, loopStart, loopEnd,
									  length, repeats);

	// Calculate the percentage of the track that has played
	percentage = (double)offset / (double)length;

	//percentage = (double)((double)curTime / (double)(length - startPoint)) + myPausePercent;
	//percentage = (double)(((double)curTime + (double)startPoint - (double)startTime) / ((double)length));
	
	if(percentage >= 1.)
	{
		return -1;
	}

	return (double)trackLength * percentage;
}

//==Set Volume Method==
void MIDI::SetMidiVolume(double volume)
{
	const MUSIC_VOLUME_RANGE = DMUS_VOLUME_MAX;
	long dxvolume = (volume / 100.) * MUSIC_VOLUME_RANGE;
	if (volume / 100. == 0)
	{
		dxvolume = DMUS_VOLUME_MIN;
	}
	else if (volume / 100. == 1)
	{
		dxvolume = DMUS_VOLUME_MAX;
	}
	myPerformanceObject->SetGlobalParam(GUID_PerfMasterVolume, 
                                        (void*)&dxvolume, sizeof(long));
}

//==Get Data Method==
BYTE* MIDI::GetData(DWORD& usedBytes)
{
	/*IDirectMusicBuffer8* buffer = NULL;
    myAudioPath->GetObjectInPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, 
                                 GUID_NULL, 0, IID_IDirectMusicBuffer, 
                                 (LPVOID*) &buffer);
	
	BYTE* data;

	HRESULT result;
	result = buffer->GetRawBufferPtr(&data);

	buffer->Release();
	
	if(result == S_OK)
	{
		return data;
	}
	else
	{
		return NULL;
	}*/

	// doesn't work :( stupid direct music
	IDirectMusicBuffer8* buffer = NULL;
	DMUS_BUFFERDESC bufferDescription = {0};
	bufferDescription.dwSize = sizeof(DMUS_BUFFERDESC);
	myDirectMusicObject->CreateMusicBuffer(&bufferDescription, &buffer, NULL);
	IDirectMusicPort8* port = NULL;
	myPerformanceObject->PChannelInfo(0, &port, NULL, NULL);
	port->Read(buffer);
	BYTE* data = NULL;
	buffer->GetRawBufferPtr(&data);
	buffer->GetUsedBytes(&usedBytes);
	buffer->Release();
	port->Release();
	return data;
}

//==Get Format Method==
WAVEFORMATEX MIDI::GetFormat()
{
	// this crap doesn't work
	IDirectMusicPort8* port = NULL;
	myPerformanceObject->PChannelInfo(0, &port, NULL, NULL);
	DWORD waveFormatSize;
	port->GetFormat(NULL, &waveFormatSize, NULL);
	WAVEFORMATEX waveFormat = {0};
	waveFormat.cbSize = waveFormatSize;
	port->GetFormat(&waveFormat, &waveFormatSize, NULL);
	port->Release();
	return waveFormat;
}

//==Get Time Offset Method==
MUSIC_TIME MIDI::GetTimeOffset(const MUSIC_TIME mtNow,           // From GetTime
							   const MUSIC_TIME mtStartTime,     // From GetStartTime
							   const MUSIC_TIME mtStartPoint,    // From GetStartPoint
                               const MUSIC_TIME mtLoopStart,     // From GetLoopPoints
                               const MUSIC_TIME mtLoopEnd,       // From GetLoopPoints
                               const MUSIC_TIME mtLength,        // From GetLength
                               const DWORD dwLoopRepeats)        // From GetRepeats
{
    // Convert mtNow from absolute time to an offset
    // from when the segment started playing.
 
    LONGLONG llOffset = mtNow - (mtStartTime - mtStartPoint);
 
    // If mtLoopEnd is not zero, set llLoopEnd to mtLoopEnd;
    // otherwise use the segment length.
 
    LONGLONG llLoopEnd = mtLoopEnd ? mtLoopEnd : mtLength;
 
    LONGLONG llLoopStart = mtLoopStart;
 
    // Adjust offset to take looping into account.
 
    if ((dwLoopRepeats != 0) &&  (llLoopStart < llLoopEnd) &&  (llLoopEnd > mtStartPoint))
    {
        if ((dwLoopRepeats != DMUS_SEG_REPEAT_INFINITE)
          &&  (llOffset > (llLoopStart + (llLoopEnd - llLoopStart) *(signed)dwLoopRepeats)))
        {
            llOffset -= (llLoopEnd - llLoopStart) * dwLoopRepeats;
        }
        else if (llOffset > llLoopStart)
        {
            llOffset = llLoopStart + (llOffset - llLoopStart) % (llLoopEnd - llLoopStart);
        }
    }

    llOffset = min(llOffset, LONG_MAX);  // LONG_MAX is defined in Limits.h.
    return long(llOffset);
}


//==Get Tempo Method==
double MIDI::GetTempo()
{
	MUSIC_TIME curTime;
	DMUS_TEMPO_PARAM tempo;
	myPerformanceObject->GetTime(NULL, &curTime);
	
	myPerformanceObject->GetParam(GUID_TempoParam, 0xFFFFFFFF,
								  DMUS_SEG_ANYTRACK, curTime,
								  NULL, &tempo);

	return tempo.dblTempo;
}

