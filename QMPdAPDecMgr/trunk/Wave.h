#include "SoundInputOutput.h"

//-----------------------------------------------------------------
// Typedefs for acessing the input decoder dlls
// See Multiplayer\\input\\wave\\wave.h for a description
// of the functions.
//-----------------------------------------------------------------

extern "C" {
	//===========================================================================
	// returns the formats can handle
	//   ie  .WAV
	// call with Position++ until Format returns ""
	// Position 10000 returns a description of the input
	//===========================================================================
	typedef void DWhatFormats(int Position, char *Format, char *Description);
	typedef DWhatFormats FAR *LPWhatFormats;
	//----------------------------------------------------------
	//===========================================================================
	// creates a new decoder object (returns a decoder object that can be passed to
	//	  all the functions)
	//
	//  FileToOpen is the filename to play
	//	ReturnError returned result (see SoundInputOutput.h)
	//  StreamsData is true if a stream of data is given (Midi is all enclosed)
	//  WFXOutputFormat should return a pointer to a WFXOutputFormat (should be WAVE_FORMAT_PCM)
	//  ProtectedStream is set if the stream is protected (and cannot say be saved to disk)
	//===========================================================================
	typedef void * DCreateANewDecoderObject(char *FileToOpen, ENOpenResult &ReturnError, bool &StreamsData, WAVEFORMATEX * *WFXOutputFormat, bool &ProtectedStream);
	typedef DCreateANewDecoderObject FAR *LPCreateANewDecoderObject;
	//----------------------------------------------------------
	//===========================================================================
	// delete a decoder object
	//===========================================================================
	typedef void DDeleteADecoderObject(void *Decoder);
	typedef DDeleteADecoderObject FAR *LPDeleteADecoderObject;
	//----------------------------------------------------------
	//===========================================================================
	// returns information on a wave file
	// NB if unable to fill an item in just leave it untouched
	//===========================================================================
	typedef void DGetTrackInfo(char *FileName, int &Returnkbps, int &ReturnFrequency, int &ReturnChannels, int &ReturnLengthmsec, int &ReturnFileSize, char *ReturnArtist, char *ReturnTrack, char *ReturnAlbum, int &ReturnYear, int &ReturnPreference, char *RetGenre);
	typedef DGetTrackInfo FAR *LPGetTrackInfo;
	//----------------------------------------------------------
	//---------------------------------------------------------------------------
	// Output is requesting data
	// if able to supply data then put into SentBuffer and return the 
	// number of bytes added to buffer
	// if unable to give data at this time, simply return 0
	// NB The BufferLen will ALLWAYS be constant for each subsequent call
	//    (this is needed for the ACM decompressor)
	// IsEOF is set to true when this is the last block of data being sent
	//
	// ALSO if this decoder does not handle Streams this function is called 
	// XX times per second so that everything can be serviced (SentBuffer is NULL)
	// just return IsEOF when playing is finished
	// dwDataStreamPosmSec is the time position of this block
	// dwStreamTotalLenghmSec is the total estimated length of the entire lot
	//---------------------------------------------------------------------------
	typedef DWORD DOutputWantsABlockOfData (void *Decoder, char *SentBuffer, DWORD BufferLen, bool &IsEOF, ENDecodeResult &DecodeResult, DWORD &dwDataStreamPosmSec, DWORD &dwStreamTotalLenghmSec);
	typedef DOutputWantsABlockOfData FAR *LPOutputWantsABlockOfData;
	//----------------------------------------------------------
	//===========================================================================
	// Called to Show at least an about page
	// Any options can be edited on this page too
	//===========================================================================
	typedef void DShowAboutOptionsPage(void);
	typedef DShowAboutOptionsPage FAR *LPShowAboutOptionsPage;
	//----------------------------------------------------------
	//===========================================================================
	// Skips to Location (0 to 100)
	//===========================================================================
	typedef void DSkipTo(void *Decoder, DWORD SkipPos);
	typedef DSkipTo FAR *LPSkipTo;
	//----------------------------------------------------------
	//===========================================================================
	// Sets the volume (position 0 to 100)
	//===========================================================================
	typedef void DSetVolume(void *Decoder, DWORD LeftVolume, DWORD RightVolume);
	typedef DSetVolume FAR *LPSetVolume;
	//----------------------------------------------------------
	//===========================================================================
	// Pauses
	//===========================================================================
	typedef void DPause(void *Decoder);
	typedef DPause FAR *LPPause;
	//----------------------------------------------------------
	//===========================================================================
	// Unpauses
	//===========================================================================
	typedef void DUnPause(void *Decoder);
	typedef DUnPause FAR *LPUnPause;
	//----------------------------------------------------------
	//===========================================================================
	// returns string information on a wave file (called by editors detail page)
	// otherwise re-open FileName
	//===========================================================================
	typedef void DGetStringInfo(char *FileName, char *RetString);
	typedef DGetStringInfo FAR *LPGetStringInfo;
	//---------------------------------------------------------------------------
	// Sets an element in the ID Tag
	// FileName is the File to change
	// Element is the name - ie ARTIST / TITLE / ALBUM / GENRE / COMMENT / YEAR
	// TagVal is the new value
	//---------------------------------------------------------------------------
	typedef void DSetIDTagElement(char *FileName, char *Element, char *TagVal);
	typedef DSetIDTagElement FAR *LPSetIDTagElement;
	//---------------------------------------------------------------------------
	// Gets an element from the ID Tag
	// FileName is the File to change
	// int Idx is a callback index, ie:
	//    Idx = 0    - this could return ARTIST
	//    Idx = 1    - this could return TITLE
	//    Idx = 2    - this has nothing more to return so RetElement is set to ""
	// RetElement is the returned name ("" when finished) - ie ARTIST / TITLE / ALBUM / GENRE / COMMENT / YEAR
	// RetTagVal is the value - "" if no tag for RetElement present
	//---------------------------------------------------------------------------
	typedef void DGetIDTagElement(char *FileName, int Idx, char *RetElement, char *RetTagVal);
	typedef DGetIDTagElement FAR *LPGetIDTagElement;
};