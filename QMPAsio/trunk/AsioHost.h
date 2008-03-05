//-----------------------------------------------------------------------------
// 
// File:	AsioHost.h
//
// About:	ASIO host interface class
//
// Author:	Ted Hess
//
//	QMP multimedia player application Software Development Kit Release 5.0.
//	Steinberg Audio Stream I/O API (c) 1997 - 2005, Steinberg Media Technologies GmbH
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

#include <asiosys.h>
#include <asio.h>
#include <asiodrivers.h>

#include "AsioOutputConvert.h"

// SpinLock timeout (reduce kernel wait calls)
#define	QMPASIO_SPINCOUNT	4000

// Channel buffer scale multiplier
// Buffer szie := <ASIO preferred size> * SCALE * CONFIG
#define BUFFER_SCALE	4

struct AsioFormatInfo
{
	DWORD	SampleRate;
	DWORD	BitsPerSample;
	DWORD	BytesPerSample;

	DWORD	BytesPerSampleTotal;
	DWORD	NChannels;
	DWORD	WaveFormat;
};

struct AsioChannelInfo
{
	BYTE			*pReader;			// Remove samples from here
	BYTE			*pWriter;			// Insert samples here
	DWORD			BufferSize;			// Total buffer size (in bytes)
	BYTE			*pChannelBuffer;
};

class CAsioHost
{
public:
	CAsioHost(void);
	~CAsioHost(void);
	
	void SetPlayerParams(DWORD SampleRate, DWORD BitsPerSample, DWORD nChannels, DWORD WaveFormat);

	BOOL OpenDevice(void);
	void CloseDevice(BOOL bRemoveDriver = TRUE);

	BOOL DoOpen(DWORD SampleRate, DWORD BitsPerSample, DWORD nChannels, DWORD WaveFormat);
	void DoWrite(WriteDataStruct *writeData);
	BOOL DoStop(int flags);
	BOOL DoPause(int flags);
	BOOL DoFlush(DWORD marker);
	BOOL DoDrain(int flags);
	void DoDrainCancel(int flags);
	void DoSetVolume(int leftlevel, int rightlevel);
	void DrainAndWait(void);

	// ASIO callbacks
	void DoSwitch(long index, ASIOBool directProcess);

	BOOL IsDriverLoaded()	{ return m_bDriverLoaded; }; 
	BOOL IsDeviceOpen()		{ return m_bDeviceOpen; }; 
	BOOL IsDeviceInited()	{ return m_bDeviceInited; };
	BOOL IsPlaying()		{ return m_bPlaying; };

	DWORD	m_nUnderruns;

protected:
	void	PlayerClose(void);
	void	PlayerStop(void);
	void	PlayerPlay(void);

	void DeviceSetup(long BufferSize);
	BOOL SetFormatInfo(DWORD SampleRate, DWORD BitsPerSample, DWORD nChannels, DWORD WaveFormat);

private:
	BOOL	m_bDriverLoaded;
	BOOL	m_bDeviceOpen;
	BOOL	m_bDeviceInited;
	BOOL	m_bOutputNotify;
	BOOL	m_bBuffersAllocated;
	BOOL	m_bPlaying;
	BOOL	m_bDraining;
	BOOL	m_bPlayerStart;

	ASIOCallbacks	m_Callbacks;
	AsioFormatInfo	m_FormatInfo;
	AsioChannelInfo	*m_pChannelInfo;
	ASIOBufferInfo	*m_pBufferInfo;

	CRITICAL_SECTION	AsioCritSec;
	HANDLE				BufferEvent;

	ASIOSampleType	m_nAsioSampleType;
	DWORD			m_nAsioBytesPerSample;
	CONVERTER_FUNC	m_pConverterFunc;

	long	m_nDeviceChannels;
	long	m_nDeviceLatency;
	long	m_nAsioBufferSize;			// ASIO device buffer size (samples)
	long	m_nChannelBufferSize;		// Input channel buffer size (samples)
	long	m_nChannelBufferLowWater;	// Trigger to wakeup writer
	long	m_nChnlBuffWriteCount;
	long	m_nChnlBuffWriteOffset;
	long	m_nChnlBuffReadOffset;

	// Player data
	DWORD	m_nPlayerSampleRate;
	DWORD	m_nPlayerBitsPerSample;
	DWORD	m_nPlayerChannels;
	DWORD	m_nPlayerSampleFormat;
	long	m_nLeftVolume;
	long	m_nRightVolume;

	friend void ClearAsioBuffer(CAsioHost *pHost, int BuferIdx, DWORD SampleOffset);
	friend ASIOError StopAsio(CAsioHost *pHost);
};
