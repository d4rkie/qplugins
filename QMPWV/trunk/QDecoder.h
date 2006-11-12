#pragma once

#include "QDecoderBase.h"

#include "wputils.h"


class QDecoder :
	public QDecoderBase, 
	public IQCDMediaDecoder
{
public:
	QDecoder(void);
	QDecoder(LPCTSTR lpszFileName);
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

public:
	virtual void __stdcall Release(void);
	virtual BOOL __stdcall StartDecoding(IQCDMediaDecoderCallback* pMDCallback, long userData);

private:
	void _format_samples (uchar *dst, int bps, long *src, unsigned long samcnt);

private:
	WavpackContext *m_wpc;

	LPBYTE sample_buffer;	// sample buffer

	TCHAR m_fn[MAX_PATH];
};

