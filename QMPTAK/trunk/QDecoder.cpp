#include "stdafx.h"
#include "QMPTAK.h"
#include "QDecoder.h"


//////////////////////////////////////////////////////////////////////////

QDecoder::QDecoder()
: QDecoderBase(L"QPlugins TAK Audio Decoder", L"1.01", L"TAK")
, m_Decoder(NULL)
, m_TAKBuf(NULL)
, m_bAbort(FALSE)
{
}

QDecoder::~QDecoder(void)
{
	Close();
}

int QDecoder::GetTrackExtents(QMediaReader & mediaReader, TrackExtents & te)
{
	TtakSSDOptions            opt  = { tak_Cpu_Any, 0 };
	TtakStreamIoInterface     sioi = { _CanRead, _CanWrite, _CanSeek, _Read, NULL, NULL, NULL, _Seek, _GetLength };
	TtakSeekableStreamDecoder ssd  = tak_SSD_Create_FromStream( &sioi, &mediaReader, &opt, NULL, NULL);
	Ttak_str_StreamInfo       si;

	if ( tak_True != tak_SSD_Valid( ssd)) {
		TtakResult ret = tak_SSD_State( ssd);
		return E_FAIL; // return ZERO duration
	}

	if ( tak_res_Ok != tak_SSD_GetStreamInfo( ssd, &si)) {
		tak_SSD_Destroy( m_Decoder);
		return E_FAIL;
	}

	te.track = 1;
	te.bytesize = mediaReader.GetSize();
	te.start = 0;
	te.end = (UINT)(si.Sizes.SampleNum * 1.0 / si.Audio.SampleRate * 1000.);
	te.unitpersec = 1000;

	tak_SSD_Destroy( ssd);

	return NOERROR;
}

int QDecoder::Open(QMediaReader & mediaReader)
{
	TtakSSDOptions            opt  = { tak_Cpu_Any, 0 };
	TtakStreamIoInterface     sioi = { _CanRead, _CanWrite, _CanSeek, _Read, NULL, NULL, NULL, _Seek, _GetLength };

	m_Decoder = tak_SSD_Create_FromStream( &sioi, &mediaReader, &opt, NULL, NULL);

	int ret = 0;
	_pmr = NULL;
	if ( tak_True == tak_SSD_Valid( m_Decoder)) {
		// get audio info
		if ( tak_res_Ok != tak_SSD_GetStreamInfo( m_Decoder, &m_StreamInfo)) {
			tak_SSD_Destroy( m_Decoder);
			m_Decoder = NULL;
			ret = 0;
		} else {
			// allocate decoding buffer
			m_TAKBuf = new BYTE[m_StreamInfo.Sizes.FrameSizeInSamples * m_StreamInfo.Audio.BlockSize];

			// save media reader
			_pmr = &mediaReader;
			_seekable = mediaReader.CanSeek();

			ret = 1;
		}
	} else {
		ret = (tak_res_ssd_Undecodable == tak_SSD_State( m_Decoder)) ? -1 : 0;
	}

	return ret;
}

int QDecoder::Close(void)
{
	if ( tak_True != tak_SSD_Valid( m_Decoder))
		return 0;

	// destroy decoding buffer
	if ( m_TAKBuf) {
		delete [] m_TAKBuf;
		m_TAKBuf = NULL;
	}

	// destroy decoder
	tak_SSD_Destroy( m_Decoder);
	m_Decoder = NULL;

	// reset state
	m_bAbort = FALSE;
	_pmr = NULL;

	return 1;
}

int QDecoder::Decode(WriteDataStruct & wd)
{
	if ( tak_True != tak_SSD_Valid( m_Decoder))
		return 0;

	if ( m_bAbort)
		return 0;

	TtakInt32 readNum;
	TtakResult takResult = tak_SSD_ReadAudio( m_Decoder, m_TAKBuf, m_StreamInfo.Sizes.FrameSizeInSamples, &readNum);
	if ( (takResult != tak_res_Ok) && (takResult != tak_res_ssd_FrameDamaged)) {
		return 0;
	} else {
		if ( readNum <= 0) {
			m_bAbort = TRUE;
			return 0;
		}

		// use the first data as the default playback format
		wd.bps = m_StreamInfo.Audio.SampleBits;
		wd.nch =  m_StreamInfo.Audio.ChannelNum;
		wd.srate = m_StreamInfo.Audio.SampleRate;
		wd.markerstart = (unsigned int)(tak_SSD_GetReadPos( m_Decoder) * 1000. / wd.srate); 
		wd.data = m_TAKBuf;;
		wd.bytelen = readNum * m_StreamInfo.Audio.BlockSize;
		wd.numsamples = readNum;

		// save the audio info
		_pos = wd.markerstart;
		_bps = wd.bps;
		_srate = wd.srate;
		_nch = wd.nch;

		return 1;
	}
}

int QDecoder::Seek(int ms)
{
	TtakResult ret;
	TtakInt64 spos;

	if ( tak_True != tak_SSD_Valid( m_Decoder))
		return 0;

	spos = (TtakInt64)(ms/1000.0 * m_StreamInfo.Audio.SampleRate);
	ret = tak_SSD_Seek( m_Decoder, spos);

	if ( tak_True != tak_SSD_Valid( m_Decoder))
		return 0;

	_pos = spos * m_StreamInfo.Audio.SampleRate * 1000.0;
	
	return 1;
}

/****************************************************************/
/* Get Waveform format struct after decoding one piece of data */
/****************************************************************/
int QDecoder::GetWaveFormFormat(WAVEFORMATEX & wfex)
{
	if ( tak_True != tak_SSD_Valid( m_Decoder))
		return 0;

	// get waveformat data from file
	do {
		Ttak_str_SimpleWaveDataHeader desc;
		if ( tak_res_Ok != tak_SSD_GetSimpleWaveDataDesc(m_Decoder, &desc))
			break;

		LPBYTE buf = new BYTE[desc.HeadSize];
		if ( tak_res_Ok !=tak_SSD_ReadSimpleWaveData( m_Decoder, buf, desc.HeadSize)) {
			delete [] buf;
			break;
		}

		// read the WAVEFORMATEX structure data
		TtakInt32 i;
		bool fnd;

		// searching the wave header data
		fnd = false;
		for ( i = 0; (i+8+4+(sizeof(wfex)-sizeof(wfex.cbSize))) < desc.HeadSize; ++i) {
			if ( buf[i+0] == 'W' && buf[i+1] == 'A' && buf[i+2] == 'V' && buf[i+3] == 'E'
			  && buf[i+4] == 'f' && buf[i+5] == 'm' && buf[i+6] == 't' && buf[i+7] == ' ')
			{
				fnd = true;
				break;
			}
		}

		if ( !fnd)
			break;

		memcpy( &wfex, buf + (i+8+4), sizeof(WAVEFORMATEX));
		wfex.cbSize = 0; // set to Zero

		delete [] buf;
		return 1;
	} while (0);

	// use default value
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
	if ( tak_True != tak_SSD_Valid( m_Decoder))
		return 0;

	ai.bitrate = tak_SSD_GetCurFrameBitRate( m_Decoder);
	ai.frequency = _srate;
	ai.mode = _nch < 2 ? 3 : (_nch > 2 ? 4 : 0);
	lstrcpyA( ai.text, "TAK");

	ai.struct_size = sizeof(AudioInfo);

	return 1;
}

int QDecoder::IsActive(void)
{
	return !m_bAbort;
}