#include ".\qdecoder.h"

#include "QInput.h"

#include "math.h"


#define MAX_NCH 2
#define MAX_BPS 32

#define BPS (WavpackGetBitsPerSample( m_wpc) <= 16 ? 16 : 32)
#define NCH (WavpackGetReducedChannels( m_wpc))
#define SAMPRATE (WavpackGetSampleRate( m_wpc))


QDecoder::QDecoder(void)
: QDecoderBase(_T("QPlugins WavPack decoder"), _T("2.1"), _T("WV"))
, m_wpc(NULL)
, sample_buffer(NULL)
{
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

	if ( !(wpc = WavpackOpenFileInput( lpszFileName, error, (cfgUI.ConfigBit & OPEN_WVC) | OPEN_TAGS | OPEN_2CH_MAX, 0)))
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
	// rememberto set m_bSeek after checking format.
	char error [128] = {'\0'};

	if ( !(m_wpc = WavpackOpenFileInput( lpszFileName, error, (cfgUI.ConfigBit & OPEN_WVC) | OPEN_TAGS | OPEN_2CH_MAX | OPEN_NORMALIZE, 23)))
		// check by parsing the returned error string.
		return (strstr( error, "not compatible") || strstr( error, "problem")) ? PLAYSTATUS_UNSUPPORTED : PLAYSTATUS_FAILED;

	play_gain = _calculate_gain ( m_wpc, &soft_clipping);

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

		// check for and handle required gain for 24-bit data
		if ( play_gain != 1.0) {
			int *gain_ptr = (int *) sample_buffer;
			int gain_cnt = tsamples;

			if ( play_gain > 1.0)
				while ( gain_cnt--) {
					float value = *gain_ptr * play_gain * (1/256.0);

					if ( soft_clipping && value > 6291456.0)
						*gain_ptr = 8388608.0 - (4398046511104.0 / (value - 4194304.0));
					else if (soft_clipping && value < -6291456.0)
						*gain_ptr = -8388608.0 - (4398046511104.0 / (value + 4194304.0));
					else if ( value > 8388607.0)
						*gain_ptr = 8388607;
					else if ( value < -8388608.0)
						*gain_ptr = -8388608;
					else
						*gain_ptr = floor (value + 0.5);

					*gain_ptr++ <<= 8;
				}
			else
				while (gain_cnt--) {
                       float value = *gain_ptr * play_gain * (1/256.0);
					*gain_ptr++ = (int) floor (value + 0.5) << 8;
				}
		}
	} else if (play_gain != 1.0) {
		short *gain_ptr = (short *) sample_buffer;
		int gain_cnt = tsamples;

		if ( play_gain > 1.0)
			while ( gain_cnt--) {
				float value = *gain_ptr * play_gain;

				if (soft_clipping && value > 24576.0)
					*gain_ptr++ = 32768.0 - (67108864.0 / (value - 16384.0));
				else if (soft_clipping && value < -24576.0)
					*gain_ptr++ = -32768.0 - (67108864.0 / (value + 16384.0));
				else if (value > 32767.0)
					*gain_ptr++ = 32767;
				else if (value < -32768.0)
					*gain_ptr++ = -32768;
				else
					*gain_ptr++ = floor (value + 0.5);
			}
		else
			while (gain_cnt--) {
				float value = *gain_ptr * play_gain;
				*gain_ptr++ = floor (value + 0.5);
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
	    while (samcnt--) {
		temp = *src++;
		dst [0] = 0;
		dst [1] = temp;
		dst += 2;
	    }

	    break;

	case 2:
	    while (samcnt--) {
		dst [0] = temp = *src++;
		dst [1] = temp >> 8;
		dst += 2;
	    }

	    break;

	case 3:
	    while (samcnt--) {
		temp = *src++;
		dst [0] = 0;
		dst [1] = temp;
		dst [2] = temp >> 8;
		dst [3] = temp >> 16;
		dst += 4;
	    }

	    break;

	case 4:
	    while (samcnt--) {
		dst [0] = temp = *src++;
		dst [1] = temp >> 8;
		dst [2] = temp >> 16;
		dst [3] = temp >> 24;
		dst += 4;
	    }

	    break;
    }
}

//////////////////////////////////////////////////////////////////////////////
// This function uses the ReplayGain mode selected by the user and the info //
// stored in the specified tag to determine the gain value used to play the //
// file and whether "soft clipping" is required. Note that the gain is in   //
// voltage scaling (not dB), so a value of 1.0 (not 0.0) is unity gain.     //
//////////////////////////////////////////////////////////////////////////////
float QDecoder::_calculate_gain(WavpackContext *wpc, int *pSoftClip)
{
	*pSoftClip = FALSE;

	if ( cfgUI.ConfigBit & (REPLAYGAIN_TRACK | REPLAYGAIN_ALBUM)) {
		float gain_value = 0.0, peak_value = 1.0;
		char value [32];

		if ((cfgUI.ConfigBit & REPLAYGAIN_ALBUM) && WavpackGetTagItem (wpc, "replaygain_album_gain", value, sizeof (value))) {
			gain_value = atof (value);

			if ( WavpackGetTagItem (wpc, "replaygain_album_peak", value, sizeof (value)))
				peak_value = atof (value);
		} else if ( WavpackGetTagItem (wpc, "replaygain_track_gain", value, sizeof (value))) {
			gain_value = atof (value);

			if ( WavpackGetTagItem (wpc, "replaygain_track_peak", value, sizeof (value)))
				peak_value = atof (value);
		} else
			return 1.0;

		// convert gain from dB to voltage (with +/- 20 dB limit)

		if (gain_value > 20.0)
			gain_value = 10.0;
		else if (gain_value < -20.0)
			gain_value = 0.1;
		else
			gain_value = pow (10.0, gain_value / 20.0);

		if (peak_value * gain_value > 1.0) {
			if ( cfgUI.ConfigBit & PREVENT_CLIPPING)
				gain_value = 1.0 / peak_value;
			else if ( cfgUI.ConfigBit & SOFTEN_CLIPPING)
				*pSoftClip = TRUE;
		}

		return gain_value;
	} else
		return 1.0;
}

