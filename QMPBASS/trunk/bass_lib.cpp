#include ".\bass_lib.h"

#include "QCDBASS.h"
#include "BASSCfgUI.h"

#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

#include "tags.h"


//-- for common functions
void load_addons(const char * fldr)
{
	if (PathFileExists(fldr)) {
		// look for plugins (in the executable's directory)
		WIN32_FIND_DATA fd;
		HANDLE fh;
		char path[MAX_PATH];

		lstrcpy(path, fldr);
		PathAppend(path, "bass*.dll");
		fh=FindFirstFile(path,&fd);
		if (fh != INVALID_HANDLE_VALUE) {
			do {
				lstrcpy(path, fldr);
				PathAppend(path, fd.cFileName);
				if (BASS_PluginLoad(path, 0)) { // plugin loaded...
					listAddons.push_back(fd.cFileName); // add file name to list
				}
			} while (FindNextFile(fh,&fd));
			FindClose(fh);
		}
	}
}

void free_addons(HPLUGIN handle)
{
	listAddons.clear();
	BASS_PluginFree(handle);
}

bool create_bass (DWORD device)
{
	if (device == 0)
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);
	else
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 100); // should be reset to default value!!

	BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT, 10000);

	BOOL ret = BASS_Init(device, nSampleRate, 0, NULL, NULL);

	load_addons(strAddonsDir); // load add-ons

	return !!ret;
}

bool destroy_bass (void)
{
	free_addons(0); // free all addons

	return BASS_Free() ? true : false;
}



//-----------------------------------------------------------------------------
// BASS Class
//-----------------------------------------------------------------------------
// Static variables
const char * bass::stream_type = NULL; // stream type, used to saving stream


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
bass::bass( const char * _path, bool _is_decode, bool _use_32fp )
: m_hBass(0)
, m_StreamFile(0)
, m_strCurTitle(0)
, use_32fp(_use_32fp)
, size(0)
, length(0)
, bitrate(0)
, is_decode(_is_decode)
, starttime(0)
, pausetime(0)
, dither(false)
, rg_gain(0), rg_peak(0), rg_scale_factor(0), hard_limiter(0), replaygain(0)
{
	for (int i = 0; i < 10; i++)
		eqfx[i] = 0;

	m_strPath = strdup(_path);

	if ( PathIsURL(m_strPath) ) {
        is_url = true;
        is_seekable = strrchr(m_strPath, '.') > strrchr(m_strPath, '/'); // internet files are also seekable

		r = NULL;
	} else {
		is_url = false;
		is_seekable = true;

		r = new_reader(m_strPath, READ);
	}
}

bass::~bass(void)
{
	if (m_hBass) {
		if (ChannelInfo.ctype & BASS_CTYPE_STREAM)
			BASS_StreamFree(m_hBass);
		else if (ChannelInfo.ctype & BASS_CTYPE_MUSIC_MOD)
			BASS_MusicFree(m_hBass);

		m_hBass = NULL;
	}

	if (m_StreamFile) {
		fclose(m_StreamFile);
		m_StreamFile = NULL;
	}

	if (r) {
		delete r;
		r = NULL;
	}

	if (m_strPath) {
		free(m_strPath);
		m_strPath = NULL;
	}

	if (m_strCurTitle) {
		free(m_strCurTitle);	// strdup'ed
		m_strCurTitle = NULL;
	}
}

//-----------------------------------------------------------------------------
// Memberfuctions
//-----------------------------------------------------------------------------

bool bass::seek(double ms)
{
	if (!is_decode) fade_volume(0, uFadeOut); // first, fade out

	bool ret;
	if (ChannelInfo.ctype & BASS_CTYPE_MUSIC_MOD) {
		ret = BASS_ChannelSetPosition(m_hBass, MAKELONG((unsigned int)(ms/1000), 0xffff)) ? true : false;
		starttime = timeGetTime() - (DWORD)ms;
	} else {
		ret = BASS_ChannelSetPosition(m_hBass, BASS_ChannelSeconds2Bytes(m_hBass, (float)(ms/1000))) ? true : false;
	}


	if (!is_decode) {
		ret = BASS_ChannelPreBuf(m_hBass, 0) ? true : false;

		// then, fade-in
		fade_volume(QCDCallbacks.Service(opGetVolume, NULL, 0, 0), uFadeIn);
	}

	return ret;
}

double bass::get_current_time()
{
	if (ChannelInfo.ctype & BASS_CTYPE_MUSIC_MOD)
		return (double)(timeGetTime() - starttime);
	else
		return (double)(BASS_ChannelBytes2Seconds( m_hBass, BASS_ChannelGetPosition(m_hBass) )*1000);
}

bool bass::set_stream_buffer_length(DWORD ms)
{
	return ms == BASS_SetConfig(BASS_CONFIG_NET_BUFFER, ms);
}

void bass::update_stream_title(char* meta, bass* pBass)
{
	char *t = NULL, *u = NULL;

	if (meta) {
		if ( (t = strstr(meta, "StreamTitle='")) && (u = strstr(meta, "StreamUrl='")) ) { // found shoutcast metadata
			t = strdup(t+13);
			u = strdup(u+11);
			strchr(t, ';')[-1] = '\0';
			strchr(u, '\'')[0] = '\0';
		} else {
			for (t = meta; *t; t += strlen(meta)+1) {
				if (!strnicmp(t, "TITLE=", 6)) { // found OGG title
					t = strdup(t+6);

					break;
				}
			}
		}

		if (t && *t) {
			if (lstrcmpi(pBass->m_strCurTitle, t)) {
				if (bStreamSaving && bSaveStreamsBasedOnTitle && pBass->m_StreamFile) {
					fclose(pBass->m_StreamFile);
					pBass->m_StreamFile = NULL;

					TCHAR szPath[MAX_PATH];
					ZeroMemory(szPath, MAX_PATH);
					lstrcpy(szPath, strStreamSavingPath);
					PathAppend(szPath, t);
					szPath[lstrlen(szPath)] = '.';
					lstrcat(szPath, stream_type);

					pBass->m_StreamFile = fopen(szPath, "wb");
				}

				if (pBass->m_strCurTitle) free(pBass->m_strCurTitle);
				pBass->m_strCurTitle = strdup(t);

				if (bStreamTitle){
					QCDCallbacks.Service(opSetTrackTitle, pBass->m_strCurTitle, (long)pBass->m_strPath, DIGITAL_STREAM_MEDIA);
				}
			}

			if (u && *u)
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
	update_stream_title((char *)data, (bass*) user);
}

void CALLBACK bass::stream_status_proc(const void *buffer, DWORD length, DWORD user)
{
	bass* pBass = (bass*)user;
	if (!pBass) {
		MessageBox(0, "stream_status_proc error!", "Debug", 0);
		return;
	}

	if (buffer) {
		if (!length)
			QCDCallbacks.Service(opSetStatusMessage, (void *)buffer, TEXT_TOOLTIP, 0);
		else { // saving stream media to local files
			if (bStreamSaving) {
				if (!pBass->m_StreamFile) {
					bStreamSaving = 0; // reset it to false

					// first, check our saving path
					if (!PathFileExists(strStreamSavingPath)) {
						TCHAR szBuffer[MAX_PATH];
						if ( browse_folder(szBuffer, _T("Select a folder to saving streamed files: "), hwndPlayer) )
							lstrcpy(const_cast<LPTSTR>((LPCTSTR)strStreamSavingPath), szBuffer);
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
						PathAppend(szPath, bSaveStreamsBasedOnTitle && pBass->m_strCurTitle && *(pBass->m_strCurTitle) ? pBass->m_strCurTitle : "stream");
						szPath[lstrlen(szPath)] = '.';
						lstrcat(szPath, stream_type);

						if ( (pBass->m_StreamFile = fopen(szPath, "wb")) ) {
                            bStreamSaving = 1;

							fwrite(buffer, sizeof(char), length, pBass->m_StreamFile);
						} else
							bStreamSaving = 0;
					} else
						bStreamSaving = 0;

					SendMessage(hwndStreamSavingBar, WM_INITDIALOG, 0, 0);
				} else
					fwrite(buffer, sizeof(char), length, pBass->m_StreamFile);
			} else {
				if (pBass->m_StreamFile) {
					fclose(pBass->m_StreamFile);
					pBass->m_StreamFile = NULL;
				}
			}
		}
	}
}

// First check the "frame sync" bits to check that you've got a frame header, and then check the relevant bits to get the bitrate.
// Note that the bit positions in that link are assuming a big-endian system.
DWORD bass::get_bitrate(void)
{
	if (!is_url && ChannelInfo.ctype & 0x1007 && r) { // VBR for mp1, mp2, mp3 local files
		DWORD framehead;
		DWORD fpos = BASS_StreamGetFilePosition(m_hBass, BASS_FILEPOS_DECODE) + BASS_StreamGetFilePosition(m_hBass, BASS_FILEPOS_START);
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

//-----------------------------------------------------------------------------
// Replaygain functions
//-----------------------------------------------------------------------------
void bass::init_rg()
{
	double track_gain = 0.0, track_peak = 0.0, album_gain = 0.0, album_peak = 0.0;
	replaygain = false;

	if (uReplayGainMode != 0 && !is_url) { // replaygain for local media
		file_info info;
		if( read_tags(r, &info) ) {
			track_gain = info.meta_get_float("replaygain_track_gain");
			track_peak = info.meta_get_float("replaygain_track_peak");
			album_gain = info.meta_get_float("replaygain_album_gain");
			album_peak = info.meta_get_float("replaygain_album_peak");

			if (track_gain || track_peak || album_gain || album_peak)
				replaygain = true;
		}

		if (uReplayGainMode == 1 ) { // track gain
			rg_gain = track_gain ? track_gain : album_gain;
			rg_peak = track_peak ? track_peak : album_peak;
		} else if (uReplayGainMode == 2) { // album gain
			rg_gain = album_gain ? album_gain : track_gain;
			rg_peak = album_peak ? album_peak : track_peak;
		}
	} else { // replaygain for local media without rg info or stream media
		rg_gain = 0.0;
		rg_peak = 0.0;
	}

	dither = !!bDither;
	FLAC__replaygain_synthesis__init_dither_context(&dither_context, use_32fp ? 32 : 16, uNoiseShaping);
	// dLog << "\ttrack_gain: " << track_gain << "\n\ttrack_peak: " << track_peak << "\n\talbum_gain: " << album_gain << "\n\talbum_peak: " << album_peak << "\n";
}

// Static function
void CALLBACK bass::rg_dsp_proc(HDSP handle, DWORD channel, void *buffer, DWORD length, DWORD user)
{
	bass* pBass = (bass*)user;
	if (!pBass) {
		MessageBox(0, "rg_dsp_proc error!", "Debug", 0);
		return;
	}

	int nch = pBass->get_nch();
	int bps = pBass->get_bps();

	// calculate rg scale factor dynamically
	pBass->hard_limiter = !!bHardLimiter;
	pBass->rg_scale_factor = FLAC__replaygain_synthesis__compute_scale_factor(pBass->rg_peak, pBass->rg_gain, (double)nPreAmp, !(pBass->hard_limiter));
	FLAC__replaygain_synthesis__apply_gain_normal(
		(FLAC__byte *)buffer, 
		(FLAC__byte *)buffer, 
		pBass->use_32fp, 
		length/(nch*bps/8), 
		nch, 
		bps, 
		pBass->rg_scale_factor, 
		pBass->hard_limiter
		);
}

//-----------------------------------------------------------------------------

int bass::init ( bool fullinit )
{
	if (is_url) { // for stream media
		size = 0;
		length = 0;

		if (fullinit) {
			QCDCallbacks.Service(opSetStatusMessage, "connecting...", TEXT_TOOLTIP, 0);

			if ( (m_hBass = BASS_StreamCreateURL(m_strPath, 0, 
				BASS_STREAM_STATUS | BASS_STREAM_BLOCK | 
				(is_decode ? BASS_STREAM_DECODE : 0) | 
				(use_32fp ? BASS_SAMPLE_FLOAT : 0), 
				stream_status_proc, (DWORD)this)) ) {
					const char *icy=BASS_ChannelGetTags(m_hBass, BASS_TAG_ICY);
					if (icy) {
						for (;*icy;icy+=strlen(icy)+1) {
							if (!memcmp(icy, "icy-name:", 9))
								QCDCallbacks.Service(opSetTrackAlbum, (void *)(icy+9), (long)m_strPath, DIGITAL_STREAM_MEDIA);// set broadcast name as album name
							if (!memcmp(icy, "icy-br:", 7))
								bitrate = atoi(icy+7) * 1000; // get bitrate info of the broadcast
						}
					}

					update_stream_title((char *)BASS_ChannelGetTags(m_hBass, BASS_TAG_META), this);
					BASS_ChannelSetSync(m_hBass, BASS_SYNC_META, 0, &stream_title_sync, (DWORD)this);
					//QCDCallbacks.Service(opSetTrackArtist, "", (long)path, DIGITAL_STREAM_MEDIA); // no artist name is need any more

					BASS_ChannelGetInfo(m_hBass, &ChannelInfo);

					stream_type = get_type();

					init_rg(); // init replaygain

					if (!is_decode)
						BASS_ChannelSetDSP(m_hBass, &rg_dsp_proc, (DWORD)this, 1 ); // set our replaygain dsp for playback mode

					return 1;
				} else
				return BASS_ERROR_FILEFORM == BASS_ErrorGetCode() ? -1 : 0;
		} else
			return 1;
	} else { // for local file
		if ( (m_hBass = BASS_StreamCreateFile(FALSE, m_strPath, 0, 0, 
			(is_decode ? BASS_STREAM_DECODE : 0) | 
			(use_32fp ? BASS_SAMPLE_FLOAT : 0))) ) { // for mp* and wav files
				size = BASS_StreamGetFilePosition(m_hBass, BASS_FILEPOS_END);
				length = (DWORD)BASS_ChannelBytes2Seconds( m_hBass, BASS_ChannelGetLength(m_hBass) );

				BASS_ChannelGetInfo(m_hBass, &ChannelInfo);

				if (fullinit) {
					bitrate = (DWORD)(size/(125*length)+0.5 )*1000; // bitrate (bps)

					init_rg(); // init replaygain

					if (!is_decode)
						BASS_ChannelSetDSP( m_hBass, &rg_dsp_proc, (DWORD)this, 1 ); // set our replaygain dsp for playback mode
				}

				return 1;
		} else if ( (m_hBass = BASS_MusicLoad(FALSE, m_strPath, 0, 0, 
			BASS_MUSIC_RAMPS | BASS_MUSIC_CALCLEN | 
			(is_decode ? BASS_MUSIC_DECODE : 0) | 
			(use_32fp ? BASS_SAMPLE_FLOAT : 0), 0)) ) { // for mod files
				size = BASS_ChannelGetLength(m_hBass);
				length = (DWORD)BASS_ChannelBytes2Seconds(m_hBass, size);

				BASS_ChannelGetInfo(m_hBass, &ChannelInfo);

				if (fullinit) {
					init_rg(); // init replaygain

					if (!is_decode)
						BASS_ChannelSetDSP( m_hBass, &rg_dsp_proc, (DWORD)this, 1 ); // set our replaygain dsp for playback mode
				}

				return 1;
		} else
			return BASS_ERROR_FILEFORM == BASS_ErrorGetCode() ? -1 : 0;
	}
}

int bass::get_data(void *out_buffer, int *out_size)
{
	LPBYTE p = (LPBYTE)out_buffer;
	int todo = *out_size;
	while (todo && BASS_ChannelIsActive(m_hBass)) {
		int rt = BASS_ChannelGetData(m_hBass, p, todo);

		if (rt == -1) { // Decode error
			if (BASS_ErrorGetCode() == BASS_ERROR_NOPLAY) // The channel is not playing, or is stalled. When handle is a "decoding channel", this indicates that it has reached the end.
				break;	// break while loop
		}
		p += rt;
		todo -= rt;
		
		if (todo > 0 && is_url) Sleep(50);
	}

	*out_size -= todo;

	if (*out_size <= 0) return -1; // means the end of the file

	return 1;
}

int bass::decode ( void *out_buffer, int *out_size )
{
	if (!is_decode) return 0;

	LPBYTE reservoir__ = new BYTE[*out_size];

	if ( get_data(reservoir__, out_size) == -1) return -1;

	// calculate rg scale factor dynamically
	hard_limiter = !!bHardLimiter;
	if (replaygain)
		rg_scale_factor = FLAC__replaygain_synthesis__compute_scale_factor(rg_peak, rg_gain, nRGPreAmp, !hard_limiter);
	else
		rg_scale_factor = FLAC__replaygain_synthesis__compute_scale_factor(0.0, 0.0, nPreAmp, !hard_limiter);

	if (dither && use_32fp)
		*out_size = FLAC__replaygain_synthesis__apply_gain(
		(FLAC__byte *)out_buffer, 
		true, // little_endian_data_out
		get_bps() == 8, // unsigned_data_out
		reservoir__, 
		get_format() == 0x0003, // 32-bit floating-point data
		(*out_size)/(get_nch()*get_bps()/8), // # of wide samples
		get_nch(), // Channels
		get_bps(), // source_bps
		dither ? 16 : get_bps(), // target_bps
		rg_scale_factor, 
		hard_limiter, 
		dither, 
		&dither_context
		);
	else
		*out_size = FLAC__replaygain_synthesis__apply_gain_normal(
		(FLAC__byte *)out_buffer, 
		reservoir__, 
		get_format() == 0x0003, // WAVE_FORMAT_IEEE_FLOAT
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

	return BASS_ChannelPreBuf(m_hBass, 0) && BASS_ChannelPlay(m_hBass, FALSE) ? true : false;
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
		return BASS_ChannelPause(m_hBass) ? true : false;
	} else
	{
		bool ret = BASS_ChannelPlay(m_hBass, FALSE) ? true : false;
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
		return true;
	}

	//// wind the frequency down for force stop
	//if (flags == 0) {
	//	BASS_ChannelSlideAttributes(handle, 1000, -1, -101, 500);
	//	Sleep(300);
	//}

	// ...and fade-out to avoid a "click"
	fade_volume(-2, uFadeOut);

	starttime = pausetime = 0;

	return BASS_ChannelStop(m_hBass) ? true : false;
}

bool bass::is_playing(void)
{
	if (is_decode) return false;

	if (!is_url)
		return BASS_ACTIVE_STOPPED != BASS_ChannelIsActive(m_hBass) ? true : false;
	else
		return true;
}

bool bass::set_volume(int level)
{
	if (is_decode) return false;

	return BASS_ChannelSetAttributes(m_hBass, -1, level, -101) ? true : false;
}

bool bass::fade_volume(int dst_volume, unsigned int elapse)
{
	bool ret = BASS_ChannelSlideAttributes(m_hBass, -1, dst_volume, -101, elapse) ? true : false;
	if (!ret) return false;

	while (BASS_ChannelIsSliding(m_hBass)) Sleep(1);

	return true;
}

bool bass::set_eq(bool enabled, char const * bands) // default 10 bands for QCD
{
	bool ret = false;

	if (enabled) {
		int i = 0; // Moved here to preserve VC++ 6.0 compability
		for (i = 0; i < 10; i++)
            if (!eqfx[i]) eqfx[i] = BASS_ChannelSetFX(m_hBass, BASS_FX_PARAMEQ, 0);

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
	} else {
		for (int i = 0; i < 10; i++) {
			if (eqfx[i]) {
				ret = BASS_ChannelRemoveFX(m_hBass, eqfx[i]) ? true: false;
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

