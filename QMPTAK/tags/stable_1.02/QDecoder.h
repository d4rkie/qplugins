#pragma once

#include "QDecoderBase.h"

#include "tak_deco_lib.h"


//////////////////////////////////////////////////////////////////////////

class QDecoder
	: public QDecoderBase
{
public:
	QDecoder();
	virtual ~QDecoder(void);

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
	BOOL m_bAbort;

	TtakSeekableStreamDecoder m_Decoder;
	Ttak_str_StreamInfo       m_StreamInfo;

	// decoded buffer (for playback)
	LPBYTE m_TAKBuf;

private: // callbacks for I/O
	static TtakBool _CanRead(void * AUser) { return tak_True; }
	static TtakBool _CanWrite(void * AUser) { return tak_False; }
	static TtakBool _CanSeek(void * AUser) { return ((QMediaReader *)AUser)->CanSeek() ? tak_True : tak_False; }
	static TtakBool _Read(void * AUser, void * ABuf, TtakInt32 ANum, TtakInt32 * AReadNum)
	{
		QMediaReader * pmr = (QMediaReader *)AUser;
		*AReadNum = pmr->Read( (LPBYTE)ABuf, ANum);
		if ( MSERROR_SUCCESS != pmr->GetLastError() || *AReadNum < ANum) {
			return tak_False;
		} else {
			return tak_True;
		}
	}
	static TtakBool _Seek(void * AUser, TtakInt64 APos)
	{
		return ((QMediaReader *)AUser)->Seek( APos) ? tak_True : tak_False;
	}
	static TtakBool _GetLength(void * AUser, TtakInt64 * ALength)
	{
		QMediaReader * pmr = (QMediaReader *)AUser;
		*ALength = pmr->GetSize();
		if ( MSERROR_SUCCESS == pmr->GetLastError())
			return tak_True;
		else {
			*ALength = 0;
			return tak_False;
		}
	};
};

