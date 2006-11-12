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

QDecoder::QDecoder(LPCTSTR lpszFileName)
: QDecoderBase(_T("QPlugins WavPack decoder"), _T("3.0"), _T("WV"))
, m_wpc(NULL)
, sample_buffer(NULL)
{
	_fn = lpszFileName; // !!save the filename used by IQCDMediaDecoder interface!!
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
unsigned int QDecoder::GetDuration(LPCTSTR lpszFileName)
{
	WavpackContext *wpc;
	char error [128] = {'\0'};
	double len;

	if ( !(wpc = WavpackOpenFileInput( lpszFileName, error, (g_bUseWVC & OPEN_WVC) | OPEN_TAGS | OPEN_2CH_MAX, 0)))
		return -1;

	len = (int)(WavpackGetNumSamples( wpc) * 1000.0 / WavpackGetSampleRate( wpc));

	wpc = WavpackCloseFile( wpc);

	return (unsigned int)len;
}

int QDecoder::Close(void)
{
	if ( m_wpc) {
        m_wpc = WavpackCloseFile( m_wpc);
		m_wpc = NULL;
	}

	if ( sample_buffer) {
		delete [] sample_buffer;
		sample_buffer = NULL;
	}

	return 1;
}

int QDecoder::OpenFile(LPCTSTR lpszFileName)
{
	// open file for decoding
	// check format accurately, return PLAYSTATUS_UNSUPPORTED for unsupported file
	// create a decoder object for decoding which should be a member of this class
	// return PLAYSTATUS_FAILED for open failed, PLAYSTATUS_SUCCESS for OK.
	//
	// remember to set m_bSeek after checking format.
	char error [128] = {'\0'};

	if ( !(m_wpc = WavpackOpenFileInput( lpszFileName, error, (g_bUseWVC & OPEN_WVC) | OPEN_TAGS | OPEN_2CH_MAX | OPEN_NORMALIZE, 23)))
		// check by parsing the returned error string.
		return (strstr( error, "not compatible") || strstr( error, "problem")) ? PLAYSTATUS_UNSUPPORTED : PLAYSTATUS_FAILED;

	//play_gain = _calculate_gain ( m_wpc, &soft_clipping);

	sample_buffer = new BYTE[576*MAX_NCH*(MAX_BPS/8)*2];

	m_bSeek = TRUE;

	return PLAYSTATUS_SUCCESS;
}

int QDecoder::Decode(WriteDataStruct * wd)
{
	// decode data
	// remember to set _srate, _bps, _nch and _pos member of base class
	int tsamples = WavpackUnpackSamples( m_wpc, (int32_t *)sample_buffer, 576) * NCH;
	int tbytes = tsamples * (BPS/8);

	if ( !tsamples)
		return 0;

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

	wd->data = sample_buffer;
	wd->bytelen = tbytes;
	_bps = wd->bps = BPS;
	wd->markerend = 0;
	wd->markerstart = _pos;
	_nch = wd->nch = NCH;
	wd->numsamples = tsamples / wd->nch;
	_srate = wd->srate = SAMPRATE;

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

int QDecoder::GetWaveFormFormat(WAVEFORMATEX * pwf)
{
	// return the wave form format for opening playback device
	// no changed needed normally.
	if ( !pwf)
		return 0;
	else {
		pwf->wFormatTag = WAVE_FORMAT_PCM;
		pwf->nChannels  = _nch;
		pwf->nSamplesPerSec = _srate;
		pwf->wBitsPerSample = _bps;
		pwf->nBlockAlign = pwf->nChannels * pwf->wBitsPerSample / 8;
		pwf->nAvgBytesPerSec = pwf->nSamplesPerSec * pwf->nBlockAlign;
		pwf->cbSize = 0;

		return 1;
	}
}

int QDecoder::GetAudioInfo(AudioInfo * pai)
{
	// return the audio info for display when playing
	// no changed needed normally.
	if ( !pai)
		return 0;
	else {
		pai->struct_size = sizeof(AudioInfo);
		pai->frequency = _srate;
		pai->bitrate = _br;
		pai->mode = (_nch == 2) ? 0 : (_nch > 2) ? 4 :	3;
		pai->layer = 0;
		pai->level = 0;
		lstrcpyn(pai->text, "WavPack", sizeof(pai->text));

		return 1;
	}
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

