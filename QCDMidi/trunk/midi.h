//==Preprocessors==
#ifndef _MIDI_H
#define _MIDI_H

#include <windows.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <dsound.h>

//==MIDI Class==
class MIDI
{
public:
	//==Constructors==
	MIDI();

	//==Destructor==
	~MIDI();

	//==Accessor Methods==
	static unsigned int GetLength(LPCSTR medianame);
	double GetPosition(int trackLength, int playFrom);
	double GetTempo();
	void SetMidiVolume(double volume);
	BYTE* GetData(DWORD& usedBytes);
	WAVEFORMATEX GetFormat();

	//==Other Methods==
	bool InitializeMidi(HWND hwnd);
	void CloseMidi();
	bool LoadMidi(const char* medianame);
	bool TeasePlayMidi(MUSIC_TIME& length, double& tempo);
	bool PlayMidi(int playfrom, int playto, int trackLength, bool unpause = false);
	bool StopMidi();
	bool PauseMidi(bool unpause);
	MUSIC_TIME GetTimeOffset(const MUSIC_TIME mtNow,           
							 const MUSIC_TIME mtStartTime,     
							 const MUSIC_TIME mtStartPoint,    
                             const MUSIC_TIME mtLoopStart,     
                             const MUSIC_TIME mtLoopEnd,       
                             const MUSIC_TIME mtLength,      
                             const DWORD dwLoopRepeats);

private:
	//==Private Members==
	IDirectMusicLoader8* myLoaderObject;
	IDirectMusicPerformance8* myPerformanceObject;
	IDirectMusicSegment8*     mySegmentObject;
	IDirectMusicAudioPath8*   myAudioPath;
	IDirectMusicSegmentState8* mySegmentStateObject;
	IDirectMusic8* myDirectMusicObject;
	WCHAR myMediaName[MAX_PATH];
	HWND myHandle;
	MUSIC_TIME myPauseTime;
	unsigned int myLength;
	unsigned int myLengthMS;
};

#endif