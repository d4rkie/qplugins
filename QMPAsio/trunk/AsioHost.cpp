//-----------------------------------------------------------------------------
// 
// File:	AsioHost.cpp
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

#include "stdafx.h"
#include "QMPAsio.h"

//
// Local ASIO wrapper functions
//
static ASIOError StopAsio(CAsioHost *pHost);
static void BufferSwitch(long index, ASIOBool directProcess);
static void SampleRateChanged(ASIOSampleRate sRate);
static long ASIOMessages(long selector, long value, void* message, double* opt);

//-----------------------------------------------------------------------------

CAsioHost::CAsioHost(void)
{
	this->m_bDeviceInited = FALSE;
	this->m_bDriverLoaded = FALSE;
	this->m_bDeviceOpen = FALSE;
	this->m_bPlaying = FALSE;
	this->m_bDraining = FALSE;
	this->m_bPlayerStart = TRUE;
	
	this->m_nChannelBufferSize = 0;
	this->m_nChannelBufferLowWater = 0;
	this->m_nUnderruns = 0;

	this->m_Callbacks.bufferSwitch = &BufferSwitch;
	this->m_Callbacks.sampleRateDidChange = &SampleRateChanged;
	this->m_Callbacks.asioMessage = &ASIOMessages;
	this->m_Callbacks.bufferSwitchTimeInfo = NULL;		// Not used

	// Use of spinlock can avoid costly kernel wait calls (best if low contention)
	::InitializeCriticalSectionAndSpinCount(&this->AsioCritSec, QMPASIO_SPINCOUNT);
	this->BufferEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

CAsioHost::~CAsioHost(void)
{
	::DeleteCriticalSection(&this->AsioCritSec);
	::CloseHandle(this->BufferEvent);
}

//
// Open function
// Load and init device if necessary
// Setup channels from player params (SetPlayerParams)
//
BOOL CAsioHost::DoOpen(DWORD SampleRate, DWORD BitsPerSample, DWORD nChannels, DWORD WaveFormat)
{
	if (!this->m_bDeviceInited)
	{
		if (this->m_bDriverLoaded)
		{
			DeviceSetup(asioApp.m_pConfig->m_nBufferSizeFactor);
		} else {
			if (!OpenDevice())
			{
				CloseDevice();
				return FALSE;
			}
		}
	}
	
	// Remeber these in case we need to re-open for format change
	this->m_nPlayerSampleRate = SampleRate;
	this->m_nPlayerBitsPerSample = BitsPerSample;
	this->m_nPlayerChannels = nChannels;
	this->m_nPlayerSampleFormat = WaveFormat;


	// Finish init and return status
	// If player is supplying mono data (nChannels .EQ. 1) be sure to
	// init at least 2 ASIO playback channels for binaural sound.
	return SetFormatInfo(m_nPlayerSampleRate,
						 m_nPlayerBitsPerSample, 
						 (m_nPlayerChannels < 2) ? 2 : m_nPlayerChannels,	// Minimum of 2 chnls
						 m_nPlayerSampleFormat);
}

//
// Load ASIO device driver and init
//
BOOL CAsioHost::OpenDevice(void)
{
	const int nMaxDevice = asioApp.m_pAsioDrivers->asioGetNumDev();

	char sDeviceName[MAX_DEVICE_NAME_LEN];

	if ((nMaxDevice > 0) && (asioApp.m_pConfig->m_nDevice < nMaxDevice))
	{
		if (asioApp.m_pAsioDrivers->asioGetDriverName(asioApp.m_pConfig->m_nDevice, sDeviceName, MAX_DEVICE_NAME_LEN) == 0)
		{
			// Make sure COM active on this thread
			::CoInitialize(NULL);
			// Attemp to activate ASIO driver
			if (asioApp.m_pAsioDrivers->loadDriver(sDeviceName))
			{
				ASIODriverInfo	DriverInfo;

				DriverInfo.asioVersion = 2;
				DriverInfo.sysRef = asioApp.m_hQCDWin;

				if (ASIOInit(&DriverInfo) == ASE_OK)
					this->DeviceSetup(asioApp.m_pConfig->m_nBufferSizeFactor);

				this->m_bDriverLoaded = TRUE;
			}
		}
	}

	// TRUE if init finished OK
	return this->m_bDeviceInited;
}

//
// Closes and unloads driver (default := unload)
//
void CAsioHost::CloseDevice(BOOL bRemoveDriver)
{
	if (this->m_bDeviceInited)
	{
		// Closing -- clear this now
		this->m_bDeviceInited = FALSE;

		// Wake up writer if necessary
		::SetEvent(this->BufferEvent);
	
		// Reset device
		PlayerStop();
		PlayerClose();
		
		// Cleanup allocated storage
		delete[] this->m_pChannelInfo;
		delete[] this->m_pBufferInfo;
 	}

	// Remove driver if requested (default)
	if (bRemoveDriver && this->m_bDriverLoaded)
	{
		asioApp.m_pAsioDrivers->removeCurrentDriver();
		this->m_bDriverLoaded = FALSE;
	}

	return;
}

//
// Init ASIO device for playback
//
void CAsioHost::DeviceSetup(long BufferSizeFactor)
{
	m_FormatInfo.SampleRate = 0;
	m_FormatInfo.WaveFormat = -1;
	m_FormatInfo.BitsPerSample = 0;
	m_FormatInfo.NChannels = 0;

	long	nInch;
	long	nOutch;

	ASIOGetChannels(&nInch, &nOutch);

	//
	// Only care about output channels
	// Note: All channels treated equally
	//
	this->m_nDeviceChannels = nOutch;

	long	minSize;
	long	maxSize;
	long	preferredSize;
	long	granularity;

	ASIOGetBufferSize(&minSize, &maxSize, &preferredSize, &granularity);

	// Set channel buffer to requested size
	this->m_nChannelBufferSize = preferredSize * BUFFER_SCALE * BufferSizeFactor;
	this->m_nAsioBufferSize = preferredSize;

	// Low water when room for a full AsioBuffer available
	this->m_nChannelBufferLowWater = this->m_nChannelBufferSize -  (2 * this->m_nAsioBufferSize);
	if (this->m_nChannelBufferLowWater <= (this->m_nChannelBufferSize / 2))
	{
		// Small buffer, wake whenever room
		this->m_nChannelBufferLowWater = this->m_nChannelBufferSize;
	}

#if defined(_DEBUG)
	CString	fmtMsg;
	fmtMsg.Format(_T("DeviceSetup: BuffSize = %d, ASIOBuff = %d, LowWater = %d\n"), m_nChannelBufferSize, m_nAsioBufferSize, m_nChannelBufferLowWater);
	OutputDebugString(fmtMsg);
#endif

	//
	// We don't monitor input
	//
	ASIOInputMonitor	InputMonitor;

	InputMonitor.input = -1;
	InputMonitor.output = 0;
	InputMonitor.gain = 0x20000000;
	InputMonitor.state = ASIOFalse;
	InputMonitor.pan = 0x3fffffff;

	ASIOFuture(kAsioSetInputMonitor, &InputMonitor);

	this->m_pChannelInfo = new AsioChannelInfo[this->m_nDeviceChannels];

	for (int idx = 0; idx < this->m_nDeviceChannels; idx++)
	{
		memset(&this->m_pChannelInfo[idx], 0, sizeof(AsioChannelInfo));
	}

	this->m_pBufferInfo = new ASIOBufferInfo[this->m_nDeviceChannels];
	this->m_bBuffersAllocated = FALSE;

	this->m_bOutputNotify = (ASIOOutputReady() == ASE_OK);

	this->m_bDeviceInited = TRUE;

	return;
}

//
// Close channels (internal)
//
void CAsioHost::PlayerClose(void)
{
	if (this->m_bBuffersAllocated)
	{
		ASIODisposeBuffers();
		this->m_bBuffersAllocated = FALSE;
	}

	for (int idx = 0; idx < this->m_nDeviceChannels; idx++) {
		if (this->m_pChannelInfo[idx].pChannelBuffer)
		{
			delete[] this->m_pChannelInfo[idx].pChannelBuffer;
			this->m_pChannelInfo[idx].pChannelBuffer = NULL;
		}
	}

	this->m_bDeviceOpen = FALSE;

	return;
}

//
// Play function
//
void CAsioHost::PlayerPlay(void)
{
	this->m_bPlaying = TRUE;

	// Startup output
	ASIOStart();

	return;
}

//
// Stop function
//
BOOL CAsioHost::DoStop(int flags)
{
	// Stop button pressed?
	if ((flags == STOPFLAG_FORCESTOP) || (flags == STOPFLAG_SHUTDOWN))
	{
		// Stop button - force close until play hit again
		CloseDevice(FALSE);
	}
	else
	{
		// Check for "seamless" playback
		if (asioApp.m_pConfig->m_bSeamless)
		{
			// Poke writer just in case
			::SetEvent(this->BufferEvent);

			// Check end of playlist
			if (flags == STOPFLAG_ALLDONE)
			{
				// Drain it now
				DrainAndWait();

				// Wait for driver to indicate we are empty
				while (this->m_bDraining)
				{
					::WaitForSingleObject(this->BufferEvent, INFINITE);
				}

				// Stop ASIO processing
				PlayerStop();
			}
		}
		else 
		{
			// Flush if SKIP or DONE
			if (!this->m_bDraining && 
				((flags == STOPFLAG_PLAYSKIP) || (flags == STOPFLAG_PLAYDONE)))
			{
				DoFlush(0);
				// Stop only if not SKIP
				if (flags == STOPFLAG_PLAYSKIP)
					return TRUE;
			}

			// May be either between tracks or all done
			while (this->m_bDraining)
			{
				::WaitForSingleObject(this->BufferEvent, INFINITE);
			}
		
			// Not seamless, just stop
			PlayerStop();
		}
	}

	return TRUE;
}

//
// Pause function
//
BOOL CAsioHost::DoPause(int flags)
{
	if (this->m_bPlaying)
	{
		if (flags == PAUSE_ENABLED)
		{
			// Stop playing (just pause)
			StopAsio(this);
		} else {
			// Resume playing
			PlayerPlay();
		}
	} else {
		return FALSE;
	}

	return TRUE;
}

//
// Stop Player (internal)
//
void CAsioHost::PlayerStop(void)
{
#if defined(_DEBUG)
	CString fmtStr;
	fmtStr.Format(_T("-> PlayerStop called, playing = %d\n"), this->m_bPlaying);
	OutputDebugString(fmtStr);
#endif

	if (this->m_bPlaying)
	{
		// Quit playing
		StopAsio(this);

		// Not playing, not draining, start needed
		this->m_bPlaying = FALSE;
		this->m_bDraining = FALSE;
		this->m_bPlayerStart = TRUE;
#if defined(_DEBUG)
		this->m_nUnderruns = 0;
#endif
	}

	// Wake any waiters
	::SetEvent(this->BufferEvent);

	return;
}

//
// Flush all buffers, next write will resume play
//
BOOL CAsioHost::DoFlush(DWORD marker)
{
	if (this->m_bPlaying)
	{
		// Stop ASIO processing
		PlayerStop();

		::EnterCriticalSection(&this->AsioCritSec);
		
		// Zap channel buffers
		for (DWORD idx = 0; idx < this->m_FormatInfo.NChannels; idx++)
		{
			memset(this->m_pChannelInfo[idx].pChannelBuffer, 0, this->m_pChannelInfo[idx].BufferSize);
			this->m_pChannelInfo[idx].pWriter = this->m_pChannelInfo[idx].pChannelBuffer;
			this->m_pChannelInfo[idx].pReader = this->m_pChannelInfo[idx].pChannelBuffer;
		}

		// Reset pointers
		this->m_nChnlBuffWriteCount = 0;
		this->m_nChnlBuffWriteOffset = 0;
		this->m_nChnlBuffReadOffset = 0;

		// Zap Asio buffers ?
		ClearAsioBuffer(this, 0, 0);
		ClearAsioBuffer(this, 1, 0);

		::LeaveCriticalSection(&this->AsioCritSec);
	}

	return TRUE;
}
//
// Wait for channel buffer to become empty (no more player data)
//
void CAsioHost::DrainAndWait()
{
	this->m_bDraining = TRUE;
	while (this->m_bDraining && this->m_bPlaying)
	{
		::EnterCriticalSection(&this->AsioCritSec);
		
		if (this->m_nChnlBuffWriteCount == 0)
		{
			::LeaveCriticalSection(&this->AsioCritSec);
			break;
		}

		::LeaveCriticalSection(&this->AsioCritSec);

		// Yield to other threads
		::WaitForSingleObject(this->BufferEvent, INFINITE);
	}

	return;
}

//
// Wait until channel buffers empty, then return
//
BOOL CAsioHost::DoDrain(int flags)
{
	if (!asioApp.m_pConfig->m_bSeamless)
	{
		// Not seamless, just drain and return when empty
		DrainAndWait();
	}
	else
	{
		// Attempt to keep smooth playing without running dry
		::SetEvent(this->BufferEvent);
	}

	return TRUE;
}

//
// Cancel drain wait
//
void CAsioHost::DoDrainCancel(int flags)
{
	// Exit drain wait if active
	this->m_bDraining = FALSE;

	::SetEvent(this->BufferEvent);

	return;
}

//
// Set device channel volume levels
//
void CAsioHost::DoSetVolume(int leftlevel, int rightlevel)
{
	this->m_nLeftVolume = leftlevel;
	this->m_nRightVolume = rightlevel;

	return;
}

BOOL CAsioHost::SetFormatInfo(DWORD sampleRate, DWORD bitsPerSample, DWORD nChannels, DWORD waveFormat)
{
	// Reset player if we are changing sample rate or nChannels
	if ((this->m_FormatInfo.SampleRate != sampleRate) || 
		(this->m_FormatInfo.NChannels != nChannels) ||
		(this->m_FormatInfo.BitsPerSample != bitsPerSample))
	{
		// Stop and close player before re-allocating buffers
		PlayerStop();
		PlayerClose();
	}

	if (this->m_FormatInfo.SampleRate != sampleRate)
	{
		// Does device support requested sample rate?
		if(ASIOCanSampleRate(sampleRate) != ASE_OK)
		{
			CString msgTitle((LPCTSTR)IDS_ASIOERRORTITLE);
			CString msgBuf;

			msgBuf.Format(IDS_RATEERRORMSG, sampleRate);
			::MessageBox(asioApp.m_hQCDWin, msgBuf, msgTitle, (MB_ICONERROR | MB_OK));

			CloseDevice();
			return FALSE;
		}

		// Set rate
		ASIOSetSampleRate(sampleRate);
	}

	//
	// If we already have buffers, this will be false
	//
	if (!this->m_bDeviceOpen)
	{
		// For now, we assume all channels are the same!
		ASIOChannelInfo	ChannelInfo;

		ChannelInfo.channel = 0;
		ChannelInfo.isInput = ASIOFalse;

		ASIOGetChannelInfo(&ChannelInfo);

		int		ChannelBytesPerSample;

		switch (ChannelInfo.type)
		{
		case ASIOSTInt16MSB:
		case ASIOSTInt16LSB:
			ChannelBytesPerSample = 2;
			break;
		case ASIOSTInt24MSB:
		case ASIOSTInt24LSB:
			ChannelBytesPerSample = 3;
			break;
		case ASIOSTInt32MSB:
		case ASIOSTFloat32MSB:
		case ASIOSTInt32MSB16:
		case ASIOSTInt32MSB24:
		case ASIOSTInt32LSB:
		case ASIOSTFloat32LSB:
		case ASIOSTInt32LSB16:
		case ASIOSTInt32LSB24:
			ChannelBytesPerSample = 4;
			break;
		case ASIOSTFloat64MSB:
		case ASIOSTFloat64LSB:
			ChannelBytesPerSample = 8;
			break;
		// Cannot handle device type out format
		default:
			{
				CString msgTitle((LPCTSTR)IDS_ASIOERRORTITLE);
				CString msgBuf;

				msgBuf.Format(IDS_DEVICETYPEERROR, ChannelInfo.type);
				::MessageBox(asioApp.m_hQCDWin, msgBuf, msgTitle, (MB_ICONERROR | MB_OK));

				CloseDevice();
				return FALSE;
			}
		}

		this->m_nAsioSampleType = ChannelInfo.type;
		this->m_nAsioBytesPerSample = ChannelBytesPerSample;

		for (DWORD idx = 0; idx < nChannels; idx++)
		{
			this->m_pChannelInfo[idx].BufferSize = this->m_nChannelBufferSize * (bitsPerSample >> 3);
			this->m_pChannelInfo[idx].pChannelBuffer = new BYTE[this->m_pChannelInfo[idx].BufferSize];
			this->m_pChannelInfo[idx].pWriter = this->m_pChannelInfo[idx].pChannelBuffer;
			this->m_pChannelInfo[idx].pReader = this->m_pChannelInfo[idx].pChannelBuffer;

			this->m_pBufferInfo[idx].isInput = ASIOFalse;
			this->m_pBufferInfo[idx].channelNum = idx;
			this->m_pBufferInfo[idx].buffers[0] = NULL;
			this->m_pBufferInfo[idx].buffers[1] = NULL;
		}

		this->m_nChnlBuffWriteCount = 0;
		this->m_nChnlBuffWriteOffset = 0;
		this->m_nChnlBuffReadOffset = 0;

		ASIOCreateBuffers(this->m_pBufferInfo, nChannels, this->m_nAsioBufferSize, &this->m_Callbacks);
		this->m_bBuffersAllocated = TRUE;

		long	InputLatency;
		long	OutputLatency;

		ASIOGetLatencies(&InputLatency, &OutputLatency);

		this->m_nDeviceLatency = OutputLatency;
	}

	this->m_FormatInfo.SampleRate = sampleRate;
	this->m_FormatInfo.BitsPerSample = bitsPerSample;
	this->m_FormatInfo.BytesPerSample = bitsPerSample >> 3;
	this->m_FormatInfo.NChannels = nChannels;
	this->m_FormatInfo.BytesPerSampleTotal = nChannels * this->m_FormatInfo.BytesPerSample;
	this->m_FormatInfo.WaveFormat = waveFormat;

	// Zap buffers if initial open
	if (!this->m_bDeviceOpen)
	{
		ClearAsioBuffer(this, 0, 0);
		ClearAsioBuffer(this, 1, 0);
	}

	// Setup conversion dispatch
	this->m_pConverterFunc = (CONVERTER_FUNC)GetConvertToAsioFunc(this->m_nAsioSampleType, waveFormat, bitsPerSample);
	if (this->m_pConverterFunc == NULL)
	{
		CString msgTitle((LPCTSTR)IDS_CVTERRORTITLE);
		CString msgBuf;

		msgBuf.Format(IDS_CVTERRORMSG, this->m_nAsioSampleType, bitsPerSample, waveFormat);
		::MessageBox(asioApp.m_hQCDWin, msgBuf, msgTitle, (MB_ICONERROR | MB_OK));

		CloseDevice();
		return FALSE;
	}

	this->m_bDeviceOpen = TRUE;

	return TRUE;
}

void CAsioHost::DoWrite(WriteDataStruct* writeData)
{
	DWORD	writeSamples = writeData->numsamples;
	BYTE	*inputPtr = (BYTE *)writeData->data;
	int		bytesPerSample = this->m_FormatInfo.BytesPerSample;
	int		samplesToSend = writeSamples;
	BOOL	monoData = (writeData->nch == 1);

	// Copy write data to channel buffers (conversion to ASIO happens in callbacks) 
	while (this->m_bDeviceInited)
	{
		// Get amount of available space
		::EnterCriticalSection(&this->AsioCritSec);
		samplesToSend = min(writeSamples, (DWORD)(this->m_nChannelBufferSize - this->m_nChnlBuffWriteCount));
		::LeaveCriticalSection(&this->AsioCritSec);
		
		for (int k = 0; k < samplesToSend; k++)
		{
			// Check for special handling of single channel data
			if (!monoData)
			{
				// Do one sample for each channel
				for (DWORD idx = 0; idx < this->m_FormatInfo.NChannels; idx++)
				{
					switch (bytesPerSample)
					{
					// 8-bit people
					case 1:
						*(BYTE *)(this->m_pChannelInfo[idx].pWriter) = *(BYTE *)inputPtr;
						break;
					// Most MP3/CDA stuff
					case 2:
						*(WORD *)(this->m_pChannelInfo[idx].pWriter) = *(WORD *)inputPtr;
						break;
					// 24-bit converter output
					case 3:
						Copy3(this->m_pChannelInfo[idx].pWriter, inputPtr);
						break;
					// IEEE float among others
					case 4:
						*(DWORD *)(this->m_pChannelInfo[idx].pWriter) = *(DWORD *)inputPtr;
						break;
					// Just in case we have 64-bit samples!
					case 8:
						*(DWORDLONG *)(this->m_pChannelInfo[idx].pWriter) = *(DWORDLONG *)inputPtr;
						break;
					}
					this->m_pChannelInfo[idx].pWriter += bytesPerSample;
					inputPtr += bytesPerSample;
				}
			} else {
				// Copy mono data to binaural buffers
				switch (bytesPerSample)
				{
				// 8-bit people
				case 1:
					*(BYTE *)(this->m_pChannelInfo[0].pWriter) = *(BYTE *)inputPtr;
					*(BYTE *)(this->m_pChannelInfo[1].pWriter) = *(BYTE *)inputPtr;
					break;
				// Most MP3/CDA stuff
				case 2:
					*(WORD *)(this->m_pChannelInfo[0].pWriter) = *(WORD *)inputPtr;
					*(WORD *)(this->m_pChannelInfo[1].pWriter) = *(WORD *)inputPtr;
					break;
				// 24-bit converter output
				case 3:
					Copy3(this->m_pChannelInfo[0].pWriter, inputPtr);
					Copy3(this->m_pChannelInfo[1].pWriter, inputPtr);
					break;
				// IEEE float among others
				case 4:
					*(DWORD *)(this->m_pChannelInfo[0].pWriter) = *(DWORD *)inputPtr;
					*(DWORD *)(this->m_pChannelInfo[1].pWriter) = *(DWORD *)inputPtr;
					break;
				// Just in case we have 64-bit samples!
				case 8:
					*(DWORDLONG *)(this->m_pChannelInfo[0].pWriter) = *(DWORDLONG *)inputPtr;
					*(DWORDLONG *)(this->m_pChannelInfo[1].pWriter) = *(DWORDLONG *)inputPtr;
					break;
				}
				this->m_pChannelInfo[0].pWriter += bytesPerSample;
				this->m_pChannelInfo[1].pWriter += bytesPerSample;
				inputPtr += bytesPerSample;
			}

			// Advance write pointer
			this->m_nChnlBuffWriteOffset++;
			// Handle buffer wrap
			if (this->m_nChnlBuffWriteOffset == this->m_nChannelBufferSize)
			{
				this->m_nChnlBuffWriteOffset = 0;
				// Reset explict buffer write pointers
				for (DWORD idx = 0; idx < this->m_FormatInfo.NChannels; idx++)
				{
					this->m_pChannelInfo[idx].pWriter = this->m_pChannelInfo[idx].pChannelBuffer;
				}
			}
		}

		::EnterCriticalSection(&this->AsioCritSec);

		// Update count of items in channel buffers
		this->m_nChnlBuffWriteCount += samplesToSend;

		::LeaveCriticalSection(&this->AsioCritSec);

		// Start things going if necessary
		if (this->m_bDeviceInited && this->m_bPlayerStart)
		{
			// Start processing buffers		
			PlayerPlay();
			this->m_bPlayerStart = FALSE;
		}

		// Update progress and quit when all buffered
		writeSamples -= samplesToSend;
		if(writeSamples == 0)
			break;

		// Yield to other threads return when space available
		::WaitForSingleObject(this->BufferEvent, INFINITE);

		// Quit if not playing any longer
		if (!this->m_bPlaying)
			break;
	}

	return;
}

static void BufferSwitch(long index, ASIOBool directProcess)
{
	if (!asioApp.m_pAsioHost)
		return;

	return asioApp.m_pAsioHost->DoSwitch(index, directProcess);
}

void CAsioHost::DoSwitch(long index, ASIOBool directProcess)
{
	int		samplesToGet;

	::EnterCriticalSection(&this->AsioCritSec);

	// All data fit?
	if (this->m_nChnlBuffWriteCount < this->m_nAsioBufferSize)
	{
		// Partial - clear space not used
		samplesToGet = this->m_nChnlBuffWriteCount;
		ClearAsioBuffer(this, index, samplesToGet);
	} else {
		// Max amount we can process this call
		samplesToGet = this->m_nAsioBufferSize;
	}

	::LeaveCriticalSection(&this->AsioCritSec);

	if (samplesToGet)
	{
		int	samplesInBuffer = this->m_nChnlBuffReadOffset + samplesToGet;

		if(samplesInBuffer <= this->m_nChannelBufferSize)
		{
			for(DWORD idx = 0; idx < this->m_FormatInfo.NChannels; idx++)
			{
				this->m_pConverterFunc(this->m_pBufferInfo[idx].buffers[index],
									   this->m_pChannelInfo[idx].pReader,
									   samplesToGet);
			}
		} else {
			// Handle buffer wrap
			int	samplesCount1 = this->m_nChannelBufferSize - this->m_nChnlBuffReadOffset;
			int	samplesCount2 = samplesInBuffer - this->m_nChannelBufferSize;

			for (DWORD idx = 0; idx < this->m_FormatInfo.NChannels; idx++)
			{
				BYTE *asioBuffer = (BYTE *)this->m_pBufferInfo[idx].buffers[index];
				// Copy both pieces
				this->m_pConverterFunc(asioBuffer,
									   this->m_pChannelInfo[idx].pReader,
									   samplesCount1);
				this->m_pConverterFunc(asioBuffer + (samplesCount1 * this->m_nAsioBytesPerSample),
									   this->m_pChannelInfo[idx].pChannelBuffer,
									   samplesCount2);
			}
		}

		// Adjust read pointer
		this->m_nChnlBuffReadOffset = (samplesInBuffer < this->m_nChannelBufferSize) ?
						samplesInBuffer : samplesInBuffer - this->m_nChannelBufferSize;
		// Reset explict buffer read pointers
		for (DWORD idx = 0; idx < this->m_FormatInfo.NChannels; idx++)
		{
			this->m_pChannelInfo[idx].pReader = this->m_pChannelInfo[idx].pChannelBuffer + (this->m_nChnlBuffReadOffset * this->m_FormatInfo.BytesPerSample);
		}

		::EnterCriticalSection(&this->AsioCritSec);
	
		// Account for items removed
		this->m_nChnlBuffWriteCount -= samplesToGet;
		if (this->m_nChnlBuffWriteCount < this->m_nChannelBufferLowWater)
		{
			// Wake writer if below low water mark
			::SetEvent(this->BufferEvent);
		}

		::LeaveCriticalSection(&this->AsioCritSec);
	}


	if (this->m_bPlaying && (samplesToGet < this->m_nAsioBufferSize))
	{
		// Only count underrun if not trying to sync with drain/stop
		if (!this->m_bDraining)
		{
			// Bogus
			this->m_nUnderruns++;
#if defined(_DEBUG)
			if (this->m_nUnderruns < 2)
				OutputDebugString(_T("*** Underrun!\n"));
#endif
		} else {
			// Signal waiter that we are empty
			this->m_bDraining = FALSE;
			::SetEvent(this->BufferEvent);
#if defined(_DEBUG)
			this->m_nUnderruns = 0;
#endif
		}
	}

	if(this->m_bOutputNotify)
		ASIOOutputReady();

	return;
}

// Don't handle these at the moment
static void SampleRateChanged(ASIOSampleRate sRate)
{
	// asioApp.m_pAsioHost->m_bResetRequest = TRUE;

	return;
}

static long ASIOMessages(long selector, long value, void* message, double* opt)
{
	// Default is not-supported
	long	RetCode = 0;

	switch(selector) {
	// Things we respond to
	case kAsioSelectorSupported:
		RetCode = (value == kAsioEngineVersion) ||
					(value == kAsioResetRequest) ||
					(value == kAsioBufferSizeChange) ||
					(value == kAsioResyncRequest) ||
					(value == kAsioLatenciesChanged) ||
					(value == kAsioSupportsTimeInfo) ||
					(value == kAsioSupportsTimeCode) ||
					(value == kAsioSupportsInputMonitor);
		break;

	// V2.x
	case kAsioEngineVersion:
		RetCode = 2;
		break;

	// Config change - defer handling
	case kAsioResetRequest:
	case kAsioBufferSizeChange:
	case kAsioLatenciesChanged:
		// asioApp.m_pAsioHost->m_bResetRequest = TRUE;
		//RetCode = 1;
		break;

	// Driver lossage
	case kAsioResyncRequest:
		asioApp.m_pAsioHost->m_nUnderruns++;
		OutputDebugString(_T("*** ResyncRequest!\n"));
		break;

	// Not supported
	case kAsioSupportsTimeInfo:
	case kAsioSupportsTimeCode:
	case kAsioSupportsInputMonitor:
	default:
		break;
	}

	return RetCode;
}

static ASIOError StopAsio(CAsioHost *pHost)
{
	::EnterCriticalSection(&pHost->AsioCritSec);

	const ASIOError	RetCode = ASIOStop();

	ClearAsioBuffer(pHost, 0, 0);
	ClearAsioBuffer(pHost, 1, 0);

	::LeaveCriticalSection(&pHost->AsioCritSec);

	return RetCode;
}

// Clear ASIO buffer at SampleOffset
static void ClearAsioBuffer(CAsioHost *pHost, int Buferidx, DWORD SampleOffset)
{
	DWORD sampleSize = pHost->m_nAsioBufferSize - SampleOffset;
	DWORD BytesPerSample = pHost->m_nAsioBytesPerSample;

	for (DWORD idx = 0; idx < pHost->m_FormatInfo.NChannels; idx++)
	{
		// Zap buffer (zero fill)
		memset((BYTE *)(pHost->m_pBufferInfo[idx].buffers[Buferidx]) + (SampleOffset * BytesPerSample),
			   0,
			   sampleSize * BytesPerSample);
	}
}
