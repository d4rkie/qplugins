#pragma once

#include "QCDInputDLL.h"
#include "qcdhelper.h"
#include "cfg_var.h"
#include "bass_lib.h"

#define PLUGIN_VERSION "v1.0 beta8.4"

//#include "ConStream.h"
//extern ConStream dLog;

typedef struct
{
	bass		*pDecoder;
	file_info	info;
	int			killThread;
	HANDLE		thread_handle;
	char		*playingFile;
} DecoderInfo_t;

extern DecoderInfo_t decoderInfo;

// vars for config UI
extern cfg_int uPrefPage; // pref page number
extern cfg_int xPrefPos; // left side of property sheet
extern cfg_int yPrefPos; // left side of property sheet

extern cfg_string strExtensions;
extern cfg_int uDeviceNum;
extern cfg_int bUse32FP;
extern cfg_int uPriority;
extern cfg_int bEqEnabled;	// enable internal equalizer
extern cfg_int bShowVBR;	// display VBR bitrate
extern cfg_int nSampleRate;	// Output samplerate

extern cfg_int uFadeIn; // fade-in sound
extern cfg_int uFadeOut; // fade-out sound
extern cfg_int nPreAmp; // preamp
extern cfg_int nRGPreAmp; // RG_preamp
extern cfg_int bHardLimiter; // 6dB hard limiter
extern cfg_int bDither; // dither
extern cfg_int uNoiseShaping; // noise shaping
extern cfg_int uReplayGainMode; // replaygain mode

extern cfg_int uBufferLen;
extern cfg_int bStreamTitle;

extern cfg_int bStreamSaving;
extern cfg_string strStreamSavingPath;
extern cfg_int bAutoShowStreamSavingBar;
extern cfg_int bSaveStreamsBasedOnTitle; // save streams based on stream title

extern cfg_int xStreamSavingBar; // left side of stream saving bar
extern cfg_int yStreamSavingBar; // top side of stream saving bar

// handle for UI
extern HWND hwndConfig; // config property
extern HWND hwndAbout; // about dialog box
extern HWND hwndStreamSavingBar; // stream saving bar

// for plug-in menu
extern void reset_menu(void);


// common function
extern int browse_folder(LPTSTR pszFolder, LPCTSTR lpszTitle, HWND hwndOwner);