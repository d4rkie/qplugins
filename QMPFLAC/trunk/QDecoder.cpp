#include "stdafx.h"
#include "QMPFLAC.h"
#include "QDecoder.h"


//////////////////////////////////////////////////////////////////////////

QDecoder::QDecoder()
: QDecoderBase(L"QPlugins FLAC Audio Decoder", L"3.4", L"FLAC:FLA")
, FLAC::Decoder::Stream()
, m_bAbort(FALSE)
, m_FLACBuf(NULL)
, m_FLACBufSize(0)
{
}

QDecoder::~QDecoder(void)
{
	Close();
}

int QDecoder::GetTrackExtents(QMediaReader & mediaReader, TrackExtents & te)
{
	/***************************************************/
	/** We use Level2 to get the stream info.         **/
	/** So we can handle the file pointer by ourself. **/
	/***************************************************/
	FLAC::Metadata::Chain chain;
	FLAC::Metadata::Iterator iterator;
	FLAC::Metadata::StreamInfo * sidata;
	::FLAC__IOCallbacks iocb = {
		(::FLAC__IOCallback_Read)read_io_callback, 
		NULL, 
		(::FLAC__IOCallback_Seek)seek_io_callback, 
		(::FLAC__IOCallback_Tell)tell_io_callback, 
		NULL, 
		NULL
	};

	if ( !chain.read( &mediaReader, iocb, false))
		return E_FAIL; //return ZERO duration

	if ( !iterator.is_valid())
		return E_FAIL; // return ZERO duration

	iterator.init( chain);

	// seek to the stream info block
	sidata = NULL;
	do {
		if ( iterator.get_block_type() == FLAC__METADATA_TYPE_STREAMINFO) {
			sidata = dynamic_cast< FLAC::Metadata::StreamInfo * >(iterator.get_block());
			break;
		}
	} while ( iterator.next());

	if ( !sidata)
		return E_FAIL; // return ZERO duration
	else {
		double total_samples = (double)sidata->get_total_samples();
		double sample_rate = (double)sidata->get_sample_rate();

		delete sidata;

		te.track = 1;
		te.bytesize = mediaReader.GetSize();
		te.start = 0;
		te.end = (unsigned int)(FLAC__uint64)(total_samples / sample_rate * 1000.0 + .5);
		te.unitpersec = 1000;

		return NOERROR;
	}
}

int QDecoder::Open(QMediaReader & mediaReader)
{
	int ret;
	::FLAC__StreamDecoderInitStatus status;

	// prepare the decoder
	//FLAC::Decoder::Stream::set_md5_checking( false);
	//FLAC::Decoder::Stream::set_metadata_ignore_all();

	// initialize the decoder
	status = FLAC::Decoder::Stream::init();

	_pmr = NULL;
	if ( status == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		_pmr = &mediaReader; // save the opened media reader
		ret = 1;

		//FLAC::Decoder::Stream::process_until_end_of_metadata();

		_seekable = _pmr->CanSeek();
	} else if ( status == FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER) {
		ret = -1; // unsupported format
	} else {
		ret = 0;
	}

	return ret;
}

int QDecoder::Close(void)
{
	int ret;

	if ( !FLAC::Decoder::Stream::is_valid())
		return 0;

	// finish decoder
	ret = FLAC::Decoder::Stream::finish();

	// clear playback buffer
	if ( m_FLACBuf) {
		delete [] m_FLACBuf; m_FLACBuf = NULL;
		m_FLACBufSize = 0;
	}

	// clear decoding reservoir
	_clear_reservoir();

	// reset state
	m_bAbort = FALSE;
	_pmr = NULL;

	return ret;
}

int QDecoder::Decode(WriteDataStruct & wd)
{
	if ( !FLAC::Decoder::Stream::is_valid())
		return 0;

	if ( m_bAbort)
		return 0;

	// make sure we have 576 decoded samples in reservoir.
	UINT total_samples = _get_available_samples_number();
	while ( total_samples < 576) {
		if ( FLAC__STREAM_DECODER_END_OF_STREAM == FLAC::Decoder::Stream::get_state()) {
			m_bAbort = TRUE;
			break;
		}

		if ( !FLAC::Decoder::Stream::process_single()) {
			m_bAbort = TRUE;
			break;
		} else {
			total_samples = _get_available_samples_number();
		}
	}

	// get decoded data from reservoir and combine them for playback
	if ( m_FLACReservoir.size() > 0) {
		ReservoirIT it = m_FLACReservoir.begin();

		// use the first data as the default playback format
		wd.bps = it->second.header.bits_per_sample;
		wd.nch = it->second.header.channels;
		wd.srate = it->second.header.sample_rate;
		wd.markerstart = (UINT)(it->second.header.number.sample_number * 1000. / wd.srate); 

		const UINT FLACBUFSIZE = 576 * (wd.bps / 8) * wd.nch; // 576 samples at most

		// prepare the playback buffer
		if ( !m_FLACBuf || m_FLACBufSize != FLACBUFSIZE) {
			if ( m_FLACBuf) {
				delete [] m_FLACBuf;
				m_FLACBuf = NULL;
			}
			m_FLACBuf = new BYTE[FLACBUFSIZE];
			m_FLACBufSize = FLACBUFSIZE;
		}

		// combine the playback.
		// stop the combination when srate, bps and nch are different.
		// destroy the used data in the reservoir.
		int bytes_in = 0;
		int bytes_remain = FLACBUFSIZE;
		int bytes_consumed = 0;
		while ( bytes_remain > 0 && it != m_FLACReservoir.end()) {
			long blocksize_bytes = it->second.header.blocksize * (wd.bps / 8) * wd.nch;
			if ( blocksize_bytes > bytes_remain) {
				int samples_consumed;

				bytes_consumed = bytes_remain;

				MoveMemory( m_FLACBuf+bytes_in, it->first, bytes_consumed);

				// advancing
				MoveMemory( it->first, it->first+bytes_consumed, blocksize_bytes - bytes_consumed);
				samples_consumed = bytes_consumed / (wd.bps / 8) / wd.nch;
				it->second.header.blocksize -= samples_consumed;
				it->second.header.number.sample_number += samples_consumed;
			} else {
				bytes_consumed = blocksize_bytes;

				MoveMemory( m_FLACBuf, it->first, bytes_consumed);

				delete [] it->first;
				m_FLACReservoir.erase( it++);
			}

			bytes_in += bytes_consumed;
			bytes_remain -= bytes_consumed;
		}

		wd.bytelen = bytes_in;
		wd.data = m_FLACBuf;
		wd.numsamples = wd.bytelen / (wd.bps / 8) / wd.nch;

		// save the audio info
		_pos = wd.markerstart;
		_bps = wd.bps;
		_srate = wd.srate;
		_nch = wd.nch;

		return 1;
	} else {
		return 0;
	}
}

int QDecoder::Seek(int ms)
{
	if ( !FLAC::Decoder::Stream::is_valid())
		return 0;

	unsigned sr = FLAC::Decoder::Stream::get_sample_rate();
	FLAC__uint64 seek_pos =(FLAC__uint64)(ms / 1000. * sr + .5);

	if ( FLAC::Decoder::Stream::seek_absolute( seek_pos)) {
		FLAC__uint64 pos;
		FLAC::Decoder::Stream::get_decode_position( &pos);
		_clear_reservoir();

		return 1;
	} else {
		if ( FLAC__STREAM_DECODER_SEEK_ERROR == FLAC::Decoder::Stream::get_state())
			FLAC::Decoder::Stream::flush();

		return 0;
	}
}

/****************************************************************/
/* Get Waveform format struct after decoding one piece of data */
/****************************************************************/
int QDecoder::GetWaveFormFormat(WAVEFORMATEX & wfex)
{
	if ( !FLAC::Decoder::Stream::is_valid())
		return 0;

	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels  = _nch;
	wfex.nSamplesPerSec = _srate;
	wfex.wBitsPerSample = _bps;
	wfex.nBlockAlign = wfex.nChannels * wfex.wBitsPerSample / 8;
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	wfex.cbSize = 0;

	return 1;
}

/******************************************************************/
/* Get Audio information struct after decoding one piece of data  */
/******************************************************************/
int QDecoder::GetAudioInfo(AudioInfo & ai)
{
	if ( !FLAC::Decoder::Stream::is_valid())
		return 0;

	long avg_br = (long)( _pmr->GetSize() / (125.*(double)(FLAC__int64)FLAC::Decoder::Stream::get_total_samples()/(double)FLAC::Decoder::Stream::get_sample_rate()));
	ai.bitrate = avg_br;
	ai.frequency = _srate;
	ai.mode = _nch < 2 ? 3 : (_nch > 2 ? 4 : 0);
	lstrcpyA( ai.text, "FLAC");

	ai.struct_size = sizeof(AudioInfo);

	return 1;
}

int QDecoder::IsActive(void)
{
	return !m_bAbort;
}