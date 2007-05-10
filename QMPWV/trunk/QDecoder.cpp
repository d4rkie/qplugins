#include "stdafx.h"
#include "QMPWV.h"
#include "QDecoder.h"

#include "math.h"

#define MAX_NCH 2
#define MAX_BPS 32

#define BPS (WavpackGetBitsPerSample( m_wpc) <= 16 ? 16 : 32)
#define NCH (WavpackGetReducedChannels( m_wpc))
#define SAMPRATE (WavpackGetSampleRate( m_wpc))

//////////////////////////////////////////////////////////////////////////

QDecoder::QDecoder()
: QDecoderBase(_T("QPlugins WavPack decoder"), _T("4.0"), _T("WV"))
, m_wpc(NULL)
, m_wvcreader(NULL)
, m_bAbort(FALSE)
, sample_buffer(NULL)
{
	// initialize the stream reader struct
	m_wpsr.read_bytes     = _read_cb;
	m_wpsr.get_pos        = _get_pos_cb;
	m_wpsr.set_pos_abs    = _abs_seek_cb;
	m_wpsr.set_pos_rel    = _rel_seek_cb;
	m_wpsr.push_back_byte = NULL;
	m_wpsr.get_length     = _get_len_cb;
	m_wpsr.can_seek       = _can_seek_cb;
	m_wpsr.write_bytes    = NULL;
}

QDecoder::~QDecoder(void)
{
	Close();
}

//////////////////////////////////////////////////////////////////////////
// Get track duration
// This should be independent from other member function
// That means: You should create a decoder object for get the duration and
// a decoder object for playback.
// This function should no effect to other member function
//
// return <0 for unsupported format or can't get the duration. (normal -1)
// normal return the actually duration
//////////////////////////////////////////////////////////////////////////
int QDecoder::GetTrackExtents(QMediaReader & mediaReader, TrackExtents & te)
{
	WavpackContext *wpc;
	char error [128] = {'\0'};
	double len;

	char infilename[1024];
	ZeroMemory( infilename, sizeof(infilename));
	WideCharToMultiByte( CP_ACP, 0, mediaReader.GetName(), -1, infilename, sizeof(infilename), 0, 0);
	if ( !(wpc = WavpackOpenFileInput( infilename, error, (g_bUseWVC & OPEN_WVC) | OPEN_2CH_MAX, 0)))
		return E_FAIL;

	te.track = 1;
	te.bytesize = mediaReader.GetSize();
	te.start = 0;
	te.end = (int)(WavpackGetNumSamples( wpc) * 1000.0 / WavpackGetSampleRate( wpc));
	te.unitpersec = 1000;

	wpc = WavpackCloseFile( wpc);

	return NOERROR;
}

int QDecoder::Open(QMediaReader & mediaReader)
{
	// open file for decoding.
	// check format accurately, return PLAYSTATUS_UNSUPPORTED for unsupported file.
	// create a decoder object for decoding which should be a member of this class.
	// return PLAYSTATUS_FAILED for open failed, PLAYSTATUS_SUCCESS for OK..
	//
	// remember to save the passed mediaReader to _pmr before doing opening operation
	//
	// remember to set m_bSeek after checking format.
	char error [128] = {'\0'};

	// open correction file
	if ( (g_bUseWVC & OPEN_WVC) == 1) {
		m_wvcreader = new QFileReader;
		if ( !m_wvcreader || !m_wvcreader->Open( mediaReader.GetName() + L"c"))
			return PLAYSTATUS_FAILED;
	}

	// save the media reader pointer
	_pmr = &mediaReader;

	if ( !(m_wpc = WavpackOpenFileInputEx( &m_wpsr, _pmr, m_wvcreader, error, OPEN_2CH_MAX | OPEN_NORMALIZE, 23))) {
		// check by parsing the returned error string.
		return (strstr( error, "not compatible") || strstr( error, "problem")) ? PLAYSTATUS_UNSUPPORTED : PLAYSTATUS_FAILED;
	}

	//play_gain = _calculate_gain ( m_wpc, &soft_clipping);

	sample_buffer = new BYTE[576*MAX_NCH*(MAX_BPS/8)*2];

	_seekable = _pmr->CanSeek();

	return PLAYSTATUS_SUCCESS;
}

int QDecoder::Close(void)
{
	// close the decoder context
	if ( m_wpc) {
		m_wpc = WavpackCloseFile( m_wpc);
		m_wpc = NULL;
	}

	// close te correction file
	if ( m_wvcreader) {
		delete m_wvcreader;
		m_wvcreader = NULL;
	}

	// destroy the sampling buffer
	if ( sample_buffer) {
		delete [] sample_buffer;
		sample_buffer = NULL;
	}

	// reset state
	m_bAbort = FALSE;
	_pmr = NULL;

	return 1;
}

int QDecoder::Decode(WriteDataStruct & wd)
{
	if ( m_bAbort)
		return 0;

	// decode data
	// remember to set _srate, _bps, _nch and _pos member of base class
	int tsamples = WavpackUnpackSamples( m_wpc, (int32_t *)sample_buffer, 576) * NCH;
	int tbytes = tsamples * (BPS/8);

	if ( !tsamples) {
		m_bAbort = TRUE;
		return 0;
	}

	_format_samples( (uchar*)sample_buffer, WavpackGetBytesPerSample( m_wpc), (long *) sample_buffer, tsamples);

	if ( BPS == 32) {
		if ( WavpackGetMode ( m_wpc) & MODE_FLOAT) {
			long *ptr = (long *) sample_buffer;
			int fcnt = tsamples;

			while (fcnt--) {
				*ptr = (long) *(float *) ptr;

				if (*ptr > 8388607)
					*ptr++ = 8388607 << 8;
				else if (*ptr < -8388608)
					*ptr = -8388608 << 8;
				else
					*ptr++ <<= 8;
			}
		}
	}

	_pos = (int)(WavpackGetSampleIndex( m_wpc) * 1000.0 / WavpackGetSampleRate( m_wpc));

	wd.data = sample_buffer;
	wd.bytelen = tbytes;
	_bps = wd.bps = BPS;
	wd.markerend = 0;
	wd.markerstart = _pos;
	_nch = wd.nch = NCH;
	wd.numsamples = tsamples / wd.nch;
	_srate = wd.srate = SAMPRATE;

	_br = (int)(WavpackGetInstantBitrate( m_wpc) + 500.0);

	return 1;
}

int QDecoder::Seek(int ms)
{
	// process seek behavior
	// affect the decoding object.
	int len = (int)(WavpackGetNumSamples( m_wpc) * 1000.0 / WavpackGetSampleRate( m_wpc)); 

	if ( ms > len - 1000 && len > 1000)
		ms = len - 1000; // don't seek to last second

	if ( WavpackSeekSample( m_wpc, (int)(SAMPRATE / 1000.0 * ms)))
		_pos = (int)(WavpackGetSampleIndex( m_wpc) * 1000.0 / SAMPRATE);

	return 1;
}

int QDecoder::GetWaveFormFormat(WAVEFORMATEX & wfex)
{
	// return the wave form format for opening playback device
	// no changed needed normally.
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels  = _nch;
	wfex.nSamplesPerSec = _srate;
	wfex.wBitsPerSample = _bps;
	wfex.nBlockAlign = wfex.nChannels * wfex.wBitsPerSample / 8;
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	wfex.cbSize = 0;

	return 1;
}

int QDecoder::GetAudioInfo(AudioInfo & ai)
{
	// return the audio info for display when playing
	// no changed needed normally.
	ai.struct_size = sizeof(AudioInfo);
	ai.frequency = _srate;
	ai.bitrate = _br;
	ai.mode = (_nch == 2) ? 0 : (_nch > 2) ? 4 :	3;
	ai.layer = 0;
	ai.level = 0;
	lstrcpynA( ai.text, "WavPack", sizeof(ai.text));

	return 1;
}

int QDecoder::IsActive(void)
{
	return !m_bAbort;
}

//------------------------------------------------------------------------------------------

void QDecoder::_format_samples (uchar *dst, int bps, long *src, unsigned long samcnt)
{
	long temp;

	switch (bps) {
	case 1:
		while ( samcnt--) {
			temp = *src++;
			dst [0] = 0;
			dst [1] = temp;
			dst += 2;
		}

		break;
	case 2:
		while ( samcnt--) {
			dst [0] = temp = *src++;
			dst [1] = temp >> 8;
			dst += 2;
		}

		break;
	case 3:
		while ( samcnt--) {
			temp = *src++;
			dst [0] = 0;
			dst [1] = temp;
			dst [2] = temp >> 8;
			dst [3] = temp >> 16;
			dst += 4;
		}

		break;
	case 4:
		while ( samcnt--) {
			dst [0] = temp = *src++;
			dst [1] = temp >> 8;
			dst [2] = temp >> 16;
			dst [3] = temp >> 24;
			dst += 4;
		}

		break;
	}
}

