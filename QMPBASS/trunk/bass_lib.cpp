#include ".\bass_lib.h"

#include "QCDBASS.h"
#include "BASSCfgUI.h"

#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

#include "tags.h"

char * cur_title = NULL; // stream media title per read, used for stream saving


//-- for common functions
bool create_bass (DWORD device)
{
	if (device == 0)
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);
	else
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 100); // should be reset to default value!!

	BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT, 10000);

	return BASS_Init(device, 44100, 0, NULL, NULL) ? true : false;
}

bool destroy_bass (void)
{
	return BASS_Free() ? true : false;
}


//-- for bass
bass::bass( const char * _path, bool _is_decode, bool _use_32fp )
: handle(0)
, use_32fp(_use_32fp)
, size(0)
, length(0)
, bitrate(0)
, is_decode(_is_decode)
, starttime(0)
, pausetime(0)
, dither(false)
{
	for (int i = 0; i < 10; i++)
		eqfx[i] = 0;

	path = strdup(_path);

	if ( PathIsURL(path) ) {
        is_url = true;
        is_seekable = strrchr(path, '.') > strrchr(path, '/'); // internet files are also seekable

		r = NULL;
	}
	else {
		is_url = false;
		is_seekable = true;

		r = new_reader(path, READ);
	}
}

bass::~bass(void)
{
	if (handle) {
		if (info.ctype & BASS_CTYPE_STREAM) BASS_StreamFree(handle);
		else if (info.ctype & BASS_CTYPE_MUSIC_MOD) BASS_MusicFree(handle);

		handle = 0;
	}

	if (path) {
		free(path);
		path = NULL;
	}

	if (r) {
		delete r;
		r = NULL;
	}
}

bool bass::seek(double ms)
{
	if (!is_decode) fade_volume(0, uFadeOut); // first, fade out

	bool ret;
	if (info.ctype & BASS_CTYPE_MUSIC_MOD) {
		ret = BASS_ChannelSetPosition(handle, MAKELONG((unsigned int)(ms/1000), 0xffff)) ? true : false;
		starttime = timeGetTime() - (DWORD)ms;
	}
	else {
		ret = BASS_ChannelSetPosition(handle, BASS_ChannelSeconds2Bytes(handle, (float)(ms/1000))) ? true : false;
	}


	if (!is_decode) {
		ret = BASS_ChannelPreBuf(handle) ? true : false;

		// then, fade-in
		fade_volume(QCDCallbacks.Service(opGetVolume, NULL, 0, 0), uFadeIn);
	}

	return ret;
}

double bass::get_current_time()
{
	if (info.ctype & BASS_CTYPE_MUSIC_MOD)
		return (double)(timeGetTime() - starttime);
	else
		return (double)(BASS_ChannelBytes2Seconds( handle, BASS_ChannelGetPosition(handle) )*1000);
}

bool bass::set_stream_buffer_length(DWORD ms)
{
	return ms == BASS_SetConfig(BASS_CONFIG_NET_BUFFER, ms);
}

char * bass::path = NULL; // path for current media
const char * bass::stream_type = NULL; // stream type, used to saving stream
FILE * bass::fp = NULL; // file to save stream media

void bass::update_stream_title(char * meta)
{
	char *t = NULL, *u = NULL;

	if (meta) {
		if ( (t = strstr(meta, "StreamTitle='")) && (u = strstr(meta, "StreamUrl='")) ) { // found shoutcast metadata
			t = strdup(t+13);
			u = strdup(u+11);
			strchr(t, ';')[-1] = '\0';
			strchr(u, '\'')[0] = '\0';
		}
		else {
			for (t = meta; *t; t += strlen(meta)+1) {
				if (!strnicmp(t, "TITLE=", 6)) { // found OGG title
					t = strdup(t+6);

					break;
				}
			}
		}

		if (t && *t) {
			if (lstrcmpi(cur_title, t)) {
				if (bStreamSaving && bSaveStreamsBasedOnTitle && fp) {
					fclose(fp);
					fp = NULL;

					TCHAR szPath[MAX_PATH];
					ZeroMemory(szPath, MAX_PATH);
					lstrcpy(szPath, strStreamSavingPath);
					PathAppend(szPath, t);
					szPath[lstrlen(szPath)] = '.';
					lstrcat(szPath, stream_type);

					fp = fopen(szPath, "wb");
				}

				if (cur_title) free(cur_title);
				cur_title = strdup(t);

				if (bStreamTitle){
					QCDCallbacks.Service(opSetTrackTitle, cur_title, (long)path, DIGITAL_STREAM_MEDIA);
				}
			}

			if (u)
				QCDCallbacks.Service(opSetBrowserUrl, u, 0, 0);
		}
	}

	if (t) {
		free(t);
		t = NULL;
	}
	if (u) {
		free(u);
		t= NULL;
	}
}

void CALLBACK bass::stream_title_sync(HSYNC handle, DWORD channel, DWORD data, DWORD user)
{
	update_stream_title((char *)data);
}

void CALLBACK bass::stream_status_proc(void *buffer, DWORD length, DWORD user)
{
	if (buffer) {
		if (!length)
			QCDCallbacks.Service(opSetStatusMessage, buffer, TEXT_TOOLTIP, 0);
		else { // saving stream media to local files
			if (bStreamSaving) {
				if (!fp) {
					bStreamSaving = 0; // reset it to false

					// first, check our saving path
					if (!PathFileExists(strStreamSavingPath)) {
						TCHAR szBuffer[MAX_PATH];
						browse_folder(hwndPlayer, _T("Select a folder to saving streamed files: "), szBuffer);

						if(PathFileExists(szBuffer))
							lstrcpy((LPTSTR)(LPCTSTR)strStreamSavingPath, szBuffer);
					}
					// secondly, show our stream saving bar
					if (bAutoShowStreamSavingBar) {
						if (!hwndStreamSavingBar)
							hwndStreamSavingBar = DoStreamSavingBar(hInstance, hwndPlayer);
					}

					// finally, create our file
					if (PathFileExists(strStreamSavingPath)) { // now, everything should be OK!
						TCHAR szPath[MAX_PATH];
						ZeroMemory(szPath, MAX_PATH);
						lstrcpy(szPath, strStreamSavingPath);
						PathAppend(szPath, bSaveStreamsBasedOnTitle && cur_title && *cur_title ? cur_title : "stream");
						szPath[lstrlen(szPath)] = '.';
						lstrcat(szPath, stream_type);

						if ( (fp = fopen(szPath, "wb")) ) {
                            bStreamSaving = 1;

							fwrite(buffer, sizeof(char), length, fp);
						}
						else
							bStreamSaving = 0;
					}
					else
						bStreamSaving = 0;

					SendMessage(hwndStreamSavingBar, WM_INITDIALOG, 0, 0);
				}
				else
					fwrite(buffer, sizeof(char), length, fp);
			}
			else {
				if (fp) {
					fclose(fp);
					fp = NULL;
				}
			}
		}
	}
}

// First check the "frame sync" bits to check that you've got a frame header, and then check the relevant bits to get the bitrate.
// Note that the bit positions in that link are assuming a big-endian system.
DWORD bass::get_bitrate(void)
{
	if (!is_url && info.ctype&0x1007 && r) { // VBR for mp1, mp2, mp3 local files
		DWORD framehead;
		DWORD fpos = BASS_StreamGetFilePosition(handle, BASS_FILEPOS_DECODE) + BASS_StreamGetFilePosition(handle, BASS_FILEPOS_START);
		r->seek(fpos);
		r->read(&framehead, sizeof(framehead));
		framehead=((framehead&0xff)<<24)|((framehead&0xff00)<<8)
			|((framehead&0xff0000)>>8)|((framehead&0xff000000)>>24); // reverse byte order
		if ( (framehead>>21) == 0x7ff ) { // got frame sync
			int version=(framehead>>19)&3;
			int layer=(framehead>>17)&3;
			int bitrate_index=(framehead>>12)&15;

			if (version == 3 && layer == 3) // m1l1
				bitrate = bitrate_table[0][bitrate_index] * 1000;
			else if (version == 3 && layer == 2) // m1l2
				bitrate = bitrate_table[1][bitrate_index] * 1000;
			else if (version == 3 && layer == 1) // m1l3
				bitrate = bitrate_table[2][bitrate_index] * 1000;
			else if (version == 2 && layer == 3) // m2l1
				bitrate = bitrate_table[3][bitrate_index] * 1000;
			else if (version == 2 && (layer == 2 || layer == 1)) // m2l2 & m2l3
				bitrate = bitrate_table[4][bitrate_index] * 1000;
		}
	}

	return bitrate;
}

double bass::rg_gain = 0.0;
double bass::rg_peak = 0.0;
double bass::rg_scale_factor = 1.0;
bool bass::hard_limiter = false;
void bass::init_rg()
{
	double track_gain, track_peak, album_gain, album_peak;
	if (uReplayGainMode !=0 && !is_url) { // replaygain for local media
		file_info info;
		if( read_tags(r, &info) ) {
			track_gain = info.meta_get_float("replaygain_track_gain");
			track_peak = info.meta_get_float("replaygain_track_peak");
			album_gain = info.meta_get_float("replaygain_album_gain");
			album_peak = info.meta_get_float("replaygain_album_peak");
		}

		if (uReplayGainMode == 1 ) { // track gain
			rg_gain = track_gain ? track_gain : album_gain;
			rg_peak = track_peak ? track_peak : album_peak;
		}
		else if (uReplayGainMode == 2) {
			rg_gain = album_gain ? album_gain : track_gain;
			rg_peak = album_peak ? album_peak : track_peak;
		}
	}
	else { // replaygain for local media without rg info or stream media
		rg_gain = 0.0;
		rg_peak = 0.0;
	}

	dither = !!bDither;
	FLAC__replaygain_synthesis__init_dither_context(&dither_context, use_32fp ? 32 : 16, uNoiseShaping);
}

void CALLBACK bass::rg_dsp_proc(HDSP handle, DWORD channel, void *buffer, DWORD length, DWORD user)
{
	bool fp = !!LOWORD(user);
	int nch = LOBYTE(HIWORD(user));
	int bps = HIBYTE(HIWORD(user));

	// calculate rg scale factor dynamically
	hard_limiter = !!bHardLimiter;
	rg_scale_factor = FLAC__replaygain_synthesis__compute_scale_factor(rg_peak, rg_gain, (double)nPreAmp, !hard_limiter);
	FLAC__replaygain_synthesis__apply_gain_normal(
		(FLAC__byte *)buffer, 
		(FLAC__byte *)buffer, 
		fp, 
		length/(nch*bps/8), 
		nch, 
		bps, 
		rg_scale_factor, 
		hard_limiter
		);
}

int bass::init ( bool fullinit )
{
	if (is_url) { // for stream media
		size = 0;
		length = 0;

		if (fullinit) {
			QCDCallbacks.Service(opSetStatusMessage, "connecting...", TEXT_TOOLTIP, 0);

			if ( (handle = BASS_StreamCreateURL(path, 0, 
				BASS_STREAM_META | BASS_STREAM_STATUS | BASS_STREAM_BLOCK | 
				(is_decode ? BASS_STREAM_DECODE : 0) | 
				(use_32fp ? BASS_SAMPLE_FLOAT : 0), 
				stream_status_proc, 0)) ) {
					char *icy=BASS_StreamGetTags(handle, BASS_TAG_ICY);
					if (icy) {
						for (;*icy;icy+=strlen(icy)+1) {
							if (!memcmp(icy,"icy-name:",9))
								QCDCallbacks.Service(opSetTrackAlbum, icy+9, (long)path, DIGITAL_STREAM_MEDIA);// set broadcast name as album name
							if (!memcmp(icy, "icy-br:", 7))
								bitrate = atoi(icy+7) * 1000; // get bitrate info of the broadcast
						}
					}

					update_stream_title(BASS_StreamGetTags(handle, BASS_TAG_META));
					BASS_ChannelSetSync(handle, BASS_SYNC_META, 0, &stream_title_sync, 0);
					//QCDCallbacks.Service(opSetTrackArtist, "", (long)path, DIGITAL_STREAM_MEDIA); // no artist name is need any more

					BASS_ChannelGetInfo(handle, &info);

					stream_type = get_type();

					init_rg(); // init replaygain

					if (!is_decode)
						BASS_ChannelSetDSP( handle, &rg_dsp_proc, MAKELONG(use_32fp, MAKEWORD(get_nch(), get_bps())), 1 ); // set our replaygain dsp for playback mode

					return 1;
				}
			else
				return BASS_ERROR_FILEFORM == BASS_ErrorGetCode() ? -1 : 0;
		}
		else
			return 1;
	}
	else { // for local file
		if ( (handle = BASS_StreamCreateFile(FALSE, path, 0, 0, 
			(is_decode ? BASS_STREAM_DECODE : 0) | 
			(use_32fp ? BASS_SAMPLE_FLOAT : 0))) ) { // for mp* and wav files
				size = BASS_StreamGetFilePosition(handle, BASS_FILEPOS_END);
				length = (DWORD)BASS_ChannelBytes2Seconds( handle, BASS_StreamGetLength(handle) );

				if (fullinit) {
					bitrate=(DWORD)( size/(125*length)+0.5 )*1000; // bitrate (bps)

					BASS_ChannelGetInfo(handle, &info);

					init_rg(); // init replaygain

					if (!is_decode)
						BASS_ChannelSetDSP( handle, &rg_dsp_proc, MAKELONG(use_32fp, MAKEWORD(get_nch(), get_bps())), 1 ); // set our replaygain dsp for playback mode
				}

				return 1;
		}
		else if ( (handle = BASS_MusicLoad(FALSE, path, 0, 0, 
			BASS_MUSIC_RAMPS | BASS_MUSIC_CALCLEN | 
			(is_decode ? BASS_MUSIC_DECODE : 0) | 
			(use_32fp ? BASS_SAMPLE_FLOAT : 0), 0)) ) { // for mod files
				size = BASS_MusicGetLength(handle, TRUE);
				length = (DWORD)BASS_ChannelBytes2Seconds(handle, size);

				if (fullinit) {
					BASS_ChannelGetInfo(handle, &info);

					init_rg(); // init replaygain

					if (!is_decode)
						BASS_ChannelSetDSP( handle, &rg_dsp_proc, MAKELONG(use_32fp, MAKEWORD(get_nch(), get_bps())), 1 ); // set our replaygain dsp for playback mode
				}

				return 1;
		}
		else
			return BASS_ERROR_FILEFORM == BASS_ErrorGetCode() ? -1 : 0;
	}
}

int bass::get_data(void *out_buffer, int *out_size)
{
	LPBYTE p = (LPBYTE)out_buffer;
	int todo = *out_size;
	while (todo && BASS_ChannelIsActive(handle)) {
		int rt = BASS_ChannelGetData(handle, p, todo);
		p += rt;
		todo -= rt;

		if (todo > 0 && is_url) Sleep(50);
	}

	*out_size -= todo;

	if (!*out_size) return -1; // means the end of the file

	return 1;
}

int bass::decode ( void *out_buffer, int *out_size )
{
	if (!is_decode) return 0;

	LPBYTE reservoir__ = new BYTE[*out_size];

	if ( get_data(reservoir__, out_size) == -1) return -1;

	// calculate rg scale factor dynamically
	hard_limiter = !!bHardLimiter;
	rg_scale_factor = FLAC__replaygain_synthesis__compute_scale_factor(rg_peak, rg_gain, (double)nPreAmp, !hard_limiter);

	if (dither && use_32fp)
		*out_size = FLAC__replaygain_synthesis__apply_gain(
		(FLAC__byte *)out_buffer, 
		true, /* little_endian_data_out */
		get_bps() == 8, /* unsigned_data_out */
		reservoir__, 
		get_format() == 0x0003, /* 32-bit floating-point data */
		(*out_size)/(get_nch()*get_bps()/8), 
		get_nch(), 
		get_bps(), 
		dither ? 16 : get_bps(), 
		rg_scale_factor, 
		hard_limiter, 
		dither, 
		&dither_context
		);
	else
		*out_size = FLAC__replaygain_synthesis__apply_gain_normal(
		(FLAC__byte *)out_buffer, 
		reservoir__, 
		get_format() == 0x0003, 
		(*out_size)/(get_nch()*get_bps()/8), 
		get_nch(), 
		get_bps(), 
		rg_scale_factor, 
		hard_limiter
		);

	delete [] reservoir__;

	if (starttime == 0) starttime = timeGetTime(); // start timer which will be useful for MOD file

	return 1;
}

bool bass::play(void)
{
	if (is_decode) return false;

	if (starttime == 0) starttime = timeGetTime();

	return BASS_ChannelPreBuf(handle) && BASS_ChannelPlay(handle, FALSE) ? true : false;
}

bool bass::pause(int flags)
{
	if (is_decode) { // just set timer for decoding method
		if (flags)
			pausetime = timeGetTime() - starttime;
		else
			starttime = timeGetTime() - pausetime;

		return true;
	}

	if (flags) {
		// fade-out to avoid a "click"
		fade_volume(0, uFadeOut);

		pausetime = timeGetTime() - starttime;
		return BASS_ChannelPause(handle) ? true : false;
	}
	else
	{
		bool ret = BASS_ChannelPlay(handle, FALSE) ? true : false;
        starttime = timeGetTime() - pausetime;

        // fade-in
		fade_volume(QCDCallbacks.Service(opGetVolume, NULL, 0, 0), uFadeIn);

		return ret;
	}
}

bool bass::stop(int flags) // flags for force stop or playdone
{
	if (is_decode) { // just reset timer for decoding mothod
		starttime = pausetime = 0;
		return true;;
	}

	//// wind the frequency down for force stop
	//if (flags == 0) {
	//	BASS_ChannelSlideAttributes(handle, 1000, -1, -101, 500);
	//	Sleep(300);
	//}

	// ...and fade-out to avoid a "click"
	fade_volume(-2, uFadeOut);

	starttime = pausetime = 0;

	return BASS_ChannelStop(handle) ? true : false;
}

bool bass::is_playing(void)
{
	if (is_decode) return false;

	if (!is_url)
		return BASS_ACTIVE_STOPPED != BASS_ChannelIsActive(handle) ? true : false;
	else
		return true;
}

bool bass::set_volume(int level)
{
	if (is_decode) return false;

	return BASS_ChannelSetAttributes(handle, -1, level, -101) ? true : false;
}

bool bass::fade_volume(int dst_volume, unsigned int elapse)
{
	bool ret = BASS_ChannelSlideAttributes(handle, -1, dst_volume, -101, elapse) ? true : false;
	if (!ret) return false;

	while (BASS_ChannelIsSliding(handle)) Sleep(1);

	return true;
}

bool bass::set_eq(bool enabled, char const * bands) // default 10 bands for QCD
{
	bool ret = false;

	if (enabled) {
		for (int i = 0; i < 10; i++)
            if (!eqfx[i]) eqfx[i] = BASS_ChannelSetFX(handle, BASS_FX_PARAMEQ, 0);

		BASS_FXPARAMEQ eq[10];
		eq[0].fCenter = 80;
		eq[1].fCenter = 120;
		eq[2].fCenter = 250;
		eq[3].fCenter = 500;
		eq[4].fCenter = 1000;
		eq[5].fCenter = 2000;
		eq[6].fCenter = 4000;
		eq[7].fCenter = 8000;
		eq[8].fCenter = 12000;
		eq[9].fCenter = 16000;
		for (i = 0; i < 10; i++) {
			eq[i].fBandwidth = 12;
			eq[i].fGain = (float)((bands[i]-127) * 30 / 255 + 15);
		}

		for (i = 0; i < 10; i++)
			ret = BASS_FXSetParameters(eqfx[i], &eq[i]) ? true : false;
	}
	else {
		for (int i = 0; i < 10; i++) {
			if (eqfx[i]) {
				ret = BASS_ChannelRemoveFX(handle, eqfx[i]) ? true: false;
				eqfx[i] = 0;
			}
		}
	}

	return ret;
}

/* // decode stero sound to multi-stream support multi-speak
HSTREAM decoder; // decoding channel
HSTREAM player; // output stream
DWORD volume[4]; // volume levels for each speaker (0-100)

DWORD CALLBACK StreamProc(HSTREAM handle, short *buffer, DWORD length, DWORD user)
{
   int c,declen;
   short *decbuf=malloc(length/4);
   declen=BASS_ChannelGetData(decoder,decbuf,length/4); // decode some data
   for (c=0;c<declen/2;c++) {
      buffer[c*4]=decbuf[c]*volume[0]/100; // front-left
      buffer[c*4+1]=decbuf[c]*volume[1]/100; // front-right
      buffer[c*4+2]=decbuf[c]*volume[2]/100; // rear-left
      buffer[c*4+3]=decbuf[c]*volume[3]/100; // rear-right
   }
   free(decbuf);
   if (!BASS_ChannelIsActive(decoder)) // stream has ended
      return (c*8)|BASS_STREAMPROC_END;
   return c*8;
}

...

BASS_CHANNELINFO i;
decoder=BASS_StreamCreateFile(FALSE,"file.mp3",0,0,BASS_STREAM_DECODE);
BASS_ChannelGetInfo(decoder,&i);
player=BASS_StreamCreate(i.freq,4,0,(STREAMPROC*)StreamProc,0);
BASS_StreamPlay(player);
*/

