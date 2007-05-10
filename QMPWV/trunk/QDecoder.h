#pragma once

#include "QDecoderBase.h"

#include "wavpack.h"


//////////////////////////////////////////////////////////////////////////

class QDecoder : public QDecoderBase
{
public:
	QDecoder();
	~QDecoder(void);

public:
	int GetTrackExtents(QMediaReader & mediaReader, TrackExtents & te);
	int Open(QMediaReader & mediaReader);
	int Close(void);
	int Decode(WriteDataStruct & wd);
	int Seek(int ms);
	int GetWaveFormFormat(WAVEFORMATEX & wfex);
	int GetAudioInfo(AudioInfo & ai);
	int IsActive(void);

private:
	void _format_samples (uchar *dst, int bps, long *src, unsigned long samcnt);

private:
	WavpackContext * m_wpc;
	WavpackStreamReader m_wpsr;

	QFileReader * m_wvcreader; // correction file reader

	BOOL m_bAbort;

	LPBYTE sample_buffer;	// sample buffer

private:
	static int32_t _read_cb(void *id, void *data, int32_t bcount)
	{
		QMediaReader * pmr = (QMediaReader *)id;
		return pmr->Read( (LPBYTE)data, bcount);
	}
	static uint32_t _get_pos_cb(void *id)
	{
		QMediaReader * pmr = (QMediaReader *)id;
		return pmr->GetPosition();
	}
	static int _abs_seek_cb(void *id, uint32_t pos)
	{
		QMediaReader * pmr = (QMediaReader *)id;
		return pmr->Seek( pos) ? 0 : -1;
	}
	static int _rel_seek_cb(void *id, int32_t delta, int mode)
	{
		QMediaReader * pmr = (QMediaReader *)id;
		LONGLONG absolute_offset = delta;
		switch ( mode)
		{
		default:
		case SEEK_SET:
			absolute_offset = delta;
			break;
		case SEEK_CUR:
			absolute_offset = pmr->GetPosition() + delta;
			break;
		case SEEK_END:
			absolute_offset = pmr->GetSize() + delta;
			break;
		}

		return pmr->Seek( absolute_offset) ? 0 : -1;
	}
//	int (*push_back_byte)(void *id, int c);
	static uint32_t _get_len_cb(void *id)
	{
		QMediaReader * pmr = (QMediaReader *)id;
		return pmr->GetSize();
	}
	static int _can_seek_cb(void *id)
	{
		QMediaReader * pmr = (QMediaReader *)id;
		return pmr->CanSeek();
	}

	// this callback is for writing edited tags only
//	int32_t (*write_bytes)(void *id, void *data, int32_t bcount);
};

