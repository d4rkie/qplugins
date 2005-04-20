#pragma once

#include "bass.h"

#include "stdio.h"

#pragma comment(lib, "bass.lib")

#include "qcdhelper.h"
#include "replaygain_synthesis.h"


//-- command functions
bool create_bass (DWORD device);
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
	bass(const char * _path, bool _is_decode = true, bool _use_32fp = false); // default mode is decode, 'cause it need less resource
	virtual ~bass(void);
private:
	DWORD hBASS;
	DWORD eqfx[10];

	bool use_32fp;
	QWORD size;
	DWORD length;
	DWORD bitrate;
	BASS_CHANNELINFO info;

	bool is_url;
	bool is_seekable;

	reader *r;

	DWORD starttime, pausetime; // time marker for MOD file

	// replaygain
	DitherContext dither_context;
	bool dither;
private:
	static double rg_gain;
	static double rg_peak;
	static double rg_scale_factor;
	static bool hard_limiter;
	void init_rg(void);
	static void CALLBACK rg_dsp_proc(HDSP handle, DWORD channel, void *buffer, DWORD length, DWORD user);
private:
	static char * path;
	static const char * stream_type;
	static FILE * fp;
	static void update_stream_title(char * meta);
	static void CALLBACK stream_title_sync(HSYNC handle, DWORD channel, DWORD data, DWORD user);
	static void CALLBACK stream_status_proc(void *buffer, DWORD length, DWORD user);
public:
	bool is_decode; // is create for a decode module
	int init ( bool fullinit = true );
	int get_data( void *out_buffer, int *out_size);
	int decode( void *out_buffer, int *out_size );
	bool seek(double ms);
	double get_current_time(void);
	bool set_stream_buffer_length(DWORD ms);

	DWORD get_length(void) const { return length; }
	QWORD get_size(void) const { return size; }
	DWORD get_bitrate(void);
	DWORD get_nch(void) const { return info.chans; }
	DWORD get_srate(void) const { return info.freq; }
	DWORD get_bps(void) const { return info.flags & BASS_SAMPLE_FLOAT ? 32 : (info.flags & BASS_SAMPLE_8BITS) ? 8 : 16; }
	WORD get_format(void) const { return info.flags & BASS_SAMPLE_FLOAT ? 0x0003 : 1; } // 1=WAVE_FORMAT_PCM, 0x0003=WAVE_FORMAT_IEEE_FLOAT
	const char * get_type(void) const {
		switch (info.ctype)
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
		case BASS_CTYPE_MUSIC_MTM:
			return "MTM";
		case BASS_CTYPE_MUSIC_S3M:
			return "S3M";
		case BASS_CTYPE_MUSIC_XM:
			return "XM";
		case BASS_CTYPE_MUSIC_IT:
			return "IT";
		default:
			return NULL;
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
};

