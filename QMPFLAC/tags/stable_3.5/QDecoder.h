#pragma once

#include <list>
using namespace std;

#include "QDecoderBase.h"

#include "FLAC++/all.h"


//////////////////////////////////////////////////////////////////////////

class QDecoder
	: public QDecoderBase
	, private FLAC::Decoder::Stream
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

	// decoding reservoir (for saving decoded data)
	list< pair< LPBYTE, FLAC__Frame > > m_FLACReservoir;
typedef list< pair< LPBYTE, FLAC__Frame > >::iterator ReservoirIT;

	// decoded buffer (for playback)
	LPBYTE m_FLACBuf;
	UINT m_FLACBufSize;

protected: // callbacks for decoding
	virtual FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t *bytes)
	{
		if( *bytes <= 0)
			return FLAC__STREAM_DECODER_READ_STATUS_ABORT;

		DWORD bytesRead = _pmr->Read( buffer, *bytes);
		LONG error = _pmr->GetLastError();

		if ( MSERROR_SUCCESS == error) {
			*bytes = bytesRead;
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		} else if ( MSERROR_SOURCE_EXHAUSTED == error) {
			*bytes = 0;
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		} else {
			*bytes = 0;
			return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
		}
	}
	virtual FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 absolute_byte_offset)
	{
		if( _pmr->Seek( (ULONGLONG)absolute_byte_offset))
			return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
		else
			return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	}
	virtual FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64 *absolute_byte_offset)
	{
		LONGLONG offset = _pmr->GetPosition();
		if ( offset < 0)
			return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;

		*absolute_byte_offset = offset;
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	}
	virtual FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64 *stream_length)
	{
		ULONGLONG len = _pmr->GetSize();
		if ( len == 0)
			return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;

		*stream_length = len;
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}
	virtual bool eof_callback()
	{
		return _pmr->IsEOC();
	}
	virtual FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame *frame, const FLAC__int32 *const buffer[])
	{
		unsigned channel;
		int samples;

		unsigned bps = frame->header.bits_per_sample;
		unsigned nch = frame->header.channels;

		// save decoded data into the reservoir (channel-independent)
		const unsigned bytes_per_sample = bps / 8;
		const unsigned incr = bytes_per_sample * nch;

		samples = frame->header.blocksize;
		LPBYTE buf = new BYTE[samples * bytes_per_sample * nch];
		if ( !buf)
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

		// pack channel-independent data into channel-interleaved data directly
		for ( channel = 0; channel < nch; ++channel) {
			FLAC__int32 * input_ = const_cast< FLAC__int32 * >(buffer[channel]);
			LPBYTE data = buf + bytes_per_sample * channel;

			samples = frame->header.blocksize;
			while ( samples--) {
				FLAC__int32 sample = *input_++;

				switch ( bps)
				{
				case 8:
					data[0] = sample ^ 0x80;
					break;
				case 24:
					data[2] = (FLAC__byte)(sample >> 16);
					// fall through
				case 16:
					data[1] = (FLAC__byte)(sample >> 8);
					data[0] = (FLAC__byte)sample;
				}

				data += incr;
			}
		}

		m_FLACReservoir.push_back( make_pair( buf, *frame));

		return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
	}
	//virtual void metadata_callback(const FLAC__StreamMetadata *metadata) {}
	virtual void error_callback(FLAC__StreamDecoderErrorStatus status)
	{
		if ( status != FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC)
			m_bAbort = TRUE;
	}

private: // callbacks for I/O
	static size_t read_io_callback(void *ptr, size_t size, size_t nmemb, FLAC__IOHandle handle)
	{
		QMediaReader * pmr = (QMediaReader *)handle;

		return pmr->Read( (LPBYTE)ptr, size * nmemb) / size;
	}
	static int seek_io_callback(FLAC__IOHandle handle, FLAC__int64 offset, int whence)
	{
		QMediaReader * pmr = (QMediaReader *)handle;
		FLAC__int64 absolute_offset = offset;
		switch ( whence)
		{
		default:
		case SEEK_SET:
			absolute_offset = offset;
			break;
		case SEEK_CUR:
			absolute_offset = pmr->GetPosition() + offset;
			break;
		case SEEK_END:
			absolute_offset = pmr->GetSize() + offset;
			break;
		}

		return pmr->Seek(absolute_offset) ? 0 : -1;
	}
	static FLAC__int64 tell_io_callback(FLAC__IOHandle handle)
	{
		return ((QMediaReader *)handle)->GetPosition();
	}

private:
	unsigned int _get_available_samples_number()
	{
		// get total available samples number in reservoir
		unsigned int total_samples = 0;
		ReservoirIT it;
		for ( it = m_FLACReservoir.begin(); it != m_FLACReservoir.end(); ++it)
			total_samples += it->second.header.blocksize;

		return total_samples;
	}
	void _clear_reservoir()
	{
		ReservoirIT it;

		for ( it = m_FLACReservoir.begin(); it != m_FLACReservoir.end(); ++it) {
			if ( it->first) delete it->first;
		}
		m_FLACReservoir.clear();
	}
};

