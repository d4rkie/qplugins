#pragma once

#include "bass.h"

#include "stdio.h"

#pragma comment(lib, "bass.lib")

#include "qcdhelper.h"
#include "replaygain_synthesis.h"

//
// ChannelInfo types from addons
//
#define BASS_CTYPE_STREAM_CD		0x10200
#define BASS_CTYPE_STREAM_WMA		0x10300
#define BASS_CTYPE_STREAM_WMA_MP3	0x10301
#define BASS_CTYPE_STREAM_FLAC		0x10900
#define BASS_CTYPE_STREAM_FLAC_OGG	0x10901
#define BASS_CTYPE_STREAM_OFR		0x10600
#define BASS_CTYPE_STREAM_APE		0x10700
#define BASS_CTYPE_STREAM_MPC		0x10a00
#define BASS_CTYPE_STREAM_AAC		0x10b00
#define BASS_CTYPE_STREAM_MP4		0x10b01
#define BASS_CTYPE_STREAM_SPX		0x10c00
#define BASS_CTYPE_STREAM_ALAC		0x10e00
#define BASS_CTYPE_STREAM_TTA		0x10f00
#define BASS_CTYPE_STREAM_AC3		0x11000
#define BASS_CTYPE_STREAM_WV		0x10500
#define BASS_CTYPE_STREAM_WV_LH		0x10503

//-- command functions
void load_addons(const char * fldr);
void free_addons(HPLUGIN handle);
bool create_bass (DWORD device, HWND hPlayer);
bool destroy_bass (void);


//-- some stuffs
static const int  bitrate_table [5] [16] = {
    { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1 },
    { 0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, -1 },
    { 0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, -1 },
    { 0, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, -1 },
    { 0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, -1 }
};


//-- class: bass
class bass
{
public:
	// default mode is decode, 'cause it need less resource
	bass( const char * _path, bool _is_decode = true, bool _use_32fp = false );
	virtual ~bass( void );

	bool is_decode; // is create for a decode module

	int init ( bool fullinit = true );
	int get_data( void *out_buffer, int *out_size);
	int decode( void *out_buffer, int *out_size );
	bool seek(INT64 ms);
	INT64 get_current_time(void);
	void set_stream_buffer_length(DWORD ms);
	void update_stream_title(char *szMeta);
	void set_stream_title(INT64 decoder_pos);

	DWORD get_length(void) const { return length; }
	QWORD get_size(void) const { return size; }
	DWORD get_bitrate(void);
	DWORD get_nch(void) const { return ChannelInfo.chans; }
	DWORD get_srate(void) const { return ChannelInfo.freq; }
	DWORD get_bps(void) const { return ChannelInfo.flags & BASS_SAMPLE_FLOAT ? 32 : (ChannelInfo.flags & BASS_SAMPLE_8BITS) ? 8 : 16; }
	WORD get_format(void) const { return ChannelInfo.flags & BASS_SAMPLE_FLOAT ? 0x0003 : 1; } // 1=WAVE_FORMAT_PCM, 0x0003=WAVE_FORMAT_IEEE_FLOAT

	const char * get_type(void) const {
		switch (ChannelInfo.ctype)
		{
		case BASS_CTYPE_STREAM_WAV:
			return "WAV";
		case BASS_CTYPE_STREAM_OGG:
			return "OGG";
		case BASS_CTYPE_STREAM_MP1:
			return "MP1";
		case BASS_CTYPE_STREAM_MP2:
			return "MP2";
		case BASS_CTYPE_STREAM_MP3:
			return "MP3";
		case BASS_CTYPE_STREAM_AIFF:
			return "AIFF";
		case BASS_CTYPE_MUSIC_MOD:
			return "MOD";
		case BASS_CTYPE_MUSIC_MTM:
			return "MTM";
		case BASS_CTYPE_MUSIC_S3M:
			return "S3M";
		case BASS_CTYPE_MUSIC_XM:
			return "XM";
		case BASS_CTYPE_MUSIC_IT:
			return "IT";
		// check add-ons...
		case BASS_CTYPE_STREAM_CD:
			return "CDA";
		case BASS_CTYPE_STREAM_WMA:
			return "WMA";
		case BASS_CTYPE_STREAM_WMA_MP3:
			return "WMA/MP3";
		case BASS_CTYPE_STREAM_FLAC:
			return "FLAC";
		case BASS_CTYPE_STREAM_FLAC_OGG:
			return "FLAC/OGG";
		case BASS_CTYPE_STREAM_OFR:
			return "OFR/OFS";	//"Optimfrog";
		case BASS_CTYPE_STREAM_APE:
			return "APE";
		case BASS_CTYPE_STREAM_MPC:
			return "MPC";
		case BASS_CTYPE_STREAM_AAC:
			return "AAC";
		case BASS_CTYPE_STREAM_MP4:
			return "MP4";
		case BASS_CTYPE_STREAM_SPX:
			return "Speex";
		case BASS_CTYPE_STREAM_ALAC:
			return "ALAC";
		case BASS_CTYPE_STREAM_TTA:
			return "TTA";
		case BASS_CTYPE_STREAM_AC3:
			return "AC3";
		default:
			if (ChannelInfo.ctype >= BASS_CTYPE_STREAM_WV && ChannelInfo.ctype <= BASS_CTYPE_STREAM_WV_LH)
				return "Wavpack";
			else
				return "Unknown";
		}
	}

	bool can_seek(void) const { return is_seekable; }
	bool is_stream(void) const { return is_url; }

	bool play(void);
	bool pause(int flags);
	bool stop(int flags);
	bool is_playing(void);
	bool set_volume(int level);
	bool fade_volume(int dst_volume, unsigned int elapse);
	bool set_eq(bool enabled, char const * bands);

	// Streaming
	char* m_strCurTitle; // stream media title per read, used for stream saving

private:
	INT64 m_nTitleUpdateDelay;
	
	DWORD eqfx[10];
	DWORD length;
	DWORD bitrate;
	DWORD starttime, pausetime; // time marker for MOD file
	__int64 size;

	HSTREAM m_hBass;				// BASS handle
	LPBYTE m_lpDecodeReservoir;
	int m_nOut_size;
	
	BASS_CHANNELINFO ChannelInfo;

	bool is_url;
	bool is_seekable;
	bool use_32fp;

	reader_file* r;
	
	CHAR* m_strPath;

	bool resize_reservoir(int nNewSize);

	// replaygain
	DitherContext dither_context;
	bool dither;
	double rg_gain;
	double rg_peak;
	double rg_scale_factor;
	bool hard_limiter;
	bool replaygain;

	void init_rg(void);	
	static void CALLBACK rg_dsp_proc(HDSP handle, DWORD channel, void *buffer, DWORD length, void *user);

	// Streaming
	static char stream_type[16];
	FILE* m_StreamFile;

	static void CALLBACK stream_title_sync(HSYNC handle, DWORD channel, DWORD data, void *user);
	static void CALLBACK stream_status_proc(const void *buffer, DWORD length, void *user);
};