#pragma once

#include "QDecoderBase.h"

#include "wputils.h"


//////////////////////////////////////////////////////////////////////////

class QDecoder : public QDecoderBase
{
public:
	QDecoder(LPCTSTR lpszFileName = NULL);
	~QDecoder(void);

public:
	unsigned int GetDuration(LPCTSTR lpszFileName);
	int OpenStream(LPCTSTR lpszUrl, int startByte) { return PLAYSTATUS_UNSUPPORTED; } // stream unsupported!
	int OpenFile(LPCTSTR lpszFileName);
	int Close(void);
	int IsSeekable(void) { return m_bSeek; }
	int Decode(WriteDataStruct * wd);
	int Seek(int ms);
	int GetWaveFormFormat(WAVEFORMATEX * pwf);
	int GetAudioInfo(AudioInfo * pai);

private:
	void _format_samples (uchar *dst, int bps, long *src, unsigned long samcnt);

private:
	WavpackContext *m_wpc;

	LPBYTE sample_buffer;	// sample buffer
};

