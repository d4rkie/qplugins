#pragma once

#include "stdafx.h"

#include "QCDInputDLL.h"

#include "Wave.h"

#include <vector>
#include <list>
using namespace std;

typedef struct _dAP_DEC_MOD
{
	CString strFilePath;
	HMODULE hMod;

	CString strExts;
	CString strDesc;

	LPWhatFormats WhatFormats;
	LPCreateANewDecoderObject CreateANewDecoderObject;
	LPDeleteADecoderObject DeleteADecoderObject;
	LPGetTrackInfo GetTrackInfo;
	LPOutputWantsABlockOfData OutputWantsABlockOfData;
	LPShowAboutOptionsPage ShowAboutOptionsPage;
	LPSkipTo SkipTo;
	LPSetVolume SetVolume;
	LPPause Pause;
	LPUnPause UnPause;
	LPGetStringInfo GetStringInfo;
	LPSetIDTagElement SetIDTagElement;
	LPGetIDTagElement GetIDTagElement;

	UINT uOrder;
	BOOL bEnabled;
} dAPDecMod;

typedef vector< dAPDecMod >::iterator ITMod;
typedef list< ITMod > LISTModIT;

typedef struct
{
	dAPDecMod  DecMod;        // current decoder module
	void      *pDecoder;      // decoder pointer for DecMod
	LISTModIT  supModules;
	int        killThread;
	HANDLE     thread_handle;
	CString    playingFile;
} DecoderInfo_t;

extern CString g_strCacheFile;
extern CString g_strPluginDir;
extern vector< dAPDecMod > g_Modules;
extern DecoderInfo_t decoderInfo;

extern bool _load_one_dll(dAPDecMod &, const unsigned int &);
extern bool _free_one_dll(dAPDecMod &);
extern void _sort_plugins(void);

