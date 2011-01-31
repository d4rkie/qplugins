#include <algorithm> // std::transform
#include <cctype> // for toupper
#include ".\bass_lib.h"

#include "QCDBASS.h"
#include "BASSCfgUI.h"

#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

#include "tags.h"
#include <atlstr.h>

#if !defined(timeGetTime)
#define timeGetTime	GetTickCount
#endif

//-- for common functions
void load_addons(const char * fldr)
{
	if (PathFileExists(fldr)) {
		// look for plugins (in the executable's directory)
		WIN32_FIND_DATA fd;
		HANDLE fh = NULL;
		HPLUGIN hPlugin = NULL;
		char path[MAX_PATH];

		lstrcpy(path, fldr);
		PathAppend(path, "bass*.dll");
		fh=FindFirstFile(path,&fd);
		if (fh != INVALID_HANDLE_VALUE) {
			do {
				lstrcpy(path, fldr);
				PathAppend(path, fd.cFileName);
				if ((hPlugin = BASS_PluginLoad(path, 0))) { // plugin loaded...
					listAddons.push_back(fd.cFileName); // add file name to list

					// Query the extension for format types
					const BASS_PLUGININFO *info = BASS_PluginGetInfo(hPlugin);
					for (DWORD i = 0; i < info->formatc; i++)
					{
						std::string ext = info->formats[i].exts;
						// Remove all occurances of '*.' and replace ';' with ':'
						while (true)
						{
							size_t pos = ext.find("*.");
							if (pos == ext.npos)
								break;
							ext.erase(pos, 2);
							if (pos > 0)
								ext.replace(pos - 1, 1, ":");
						}
						std::transform(ext.begin(), ext.end(), ext.begin(), std::toupper);
						strAddonExtensions.append( ext );
						strAddonExtensions.append(":");
					}
				}
			} while (FindNextFile(fh,&fd));
			FindClose(fh);
			
			if (strAddonExtensions.length() > 0) // Remove last :
				strAddonExtensions = strAddonExtensions.substr(0, strAddonExtensions.length()-1);
		}
	}
}

void free_addons(HPLUGIN handle)
{
	listAddons.clear();
	BASS_PluginFree(handle);
}

bool create_bass (DWORD device, HWND hPlayer)
{
	load_addons(strAddonsDir); // load add-ons

	if (device == 0)
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 0);
	else
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 100); // should be reset to default value!!

	BASS_SetConfig(BASS_CONFIG_NET_TIMEOUT, 10000);
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, 10000);
	BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, 10000);

	BOOL ret = BASS_Init(device, nSampleRate, 0, hPlayer, NULL);

	return !!ret;
}

bool destroy_bass (void)
{
	free_addons(0); // free all addons

	return BASS_Free() ? true : false;
}

// strdup and fixup legal file name
char *normalize_file_name(char *fname)
{
	char *fname2 = (char *)malloc(strlen(fname) + 1);
	char *iptr = fname;
	char *optr = fname2;
	char ch;

	while (ch = *iptr++)
	{
		switch(ch)
		{
		case '<':
			ch = '[';
			break;
		case '>':
			ch = ']';
			break;
		case ':':
			ch = ';';
			break;
		case '"':
			ch = '\'';
			break;
		case '*':
		case '?':
			ch = '!';
			break;
		case '/':
		case '\\':
		case '|':
			ch = '_';
			break;
		}
		*optr++ = ch;
	}

	*optr = '\0';

	return fname2;
}
//-----------------------------------------------------------------------------
// BASS Class
//-----------------------------------------------------------------------------
// Static variables
char bass::stream_type[16]; // stream type, used to saving stream


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
bass::bass( const char * _path, bool _is_decode, bool _use_32fp )
: m_hBass(0)
, m_StreamFile(0)
, m_strCurTitle(0)
, m_nTitleUpdateDelay(0)
, use_32fp(_use_32fp)
, size(0)
, length(0)
, bitrate(0)
, is_decode(_is_decode)
, starttime(0)
, pausetime(0)
, dither(false)
, rg_gain(0), rg_peak(0), rg_scale_factor(0), hard_limiter(0), replaygain(0)
, m_lpDecodeReservoir(NULL), m_nOut_size(0)
{
	for (int i = 0; i < 10; i++)
		eqfx[i] = 0;

	m_strPath = _strdup(_path);

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
		if (ChannelInfo.ctype & BASS_CTYPE_STREAM) {
			if (!BASS_StreamFree(m_hBass))
				show_error("Failed on BASS_StreamFree().\nErrorcode: %d", BASS_ErrorGetCode());
		}
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

	delete [] m_lpDecodeReservoir;
}

//-----------------------------------------------------------------------------
// Memberfuctions
//-----------------------------------------------------------------------------

bool bass::seek(INT64 ms)
{
	if (!is_decode) fade_volume(0, uFadeOut); // first, fade out

	bool ret;
	if (ChannelInfo.ctype & BASS_CTYPE_MUSIC_MOD) {
		ret = BASS_ChannelSetPosition(m_hBass, MAKELONG((unsigned int)(ms / 1000), 0xffff), BASS_POS_BYTE) ? true : false;
		starttime = timeGetTime() - (DWORD)ms;
	} else {
		ret = BASS_ChannelSetPosition(m_hBass, BASS_ChannelSeconds2Bytes(m_hBass, (double)ms / 1000.0), BASS_POS_BYTE) ? true : false;
	}


	if (!is_decode) {
		ret = BASS_ChannelUpdate(m_hBass, 0) ? true : false;

		// then, fade-in
		fade_volume(QCDCallbacks.Service(opGetVolume, NULL, 0, 0), uFadeIn);
	}

	return ret;
}

INT64 bass::get_current_time()
{
	if (ChannelInfo.ctype & BASS_CTYPE_MUSIC_MOD)
		return (__int64)(timeGetTime() - starttime);
	else
		return (__int64)(1000.0 * BASS_ChannelBytes2Seconds(m_hBass, BASS_ChannelGetPosition(m_hBass, BASS_POS_BYTE)));
}

void bass::set_stream_buffer_length(DWORD ms)
{
	BASS_SetConfig(BASS_CONFIG_NET_BUFFER, ms);
	return;
}

// Input: 
void bass::update_stream_title(char *szMeta)
{
	char *t = NULL, *u = NULL;

	// Only if we have data to consume
	if (szMeta == NULL)
		return;

	if ( (t = strstr(szMeta, "StreamTitle='")) && (u = strstr(szMeta, "StreamUrl='")) ) { // found shoutcast metadata
		t = _strdup(t+13);
		u = _strdup(u+11);
		strchr(t, ';')[-1] = '\0';
		strchr(u, ';')[-1] = '\0';
	} else {
		for (t = szMeta; *t; t += strlen(szMeta)+1) {
			if (!_strnicmp(t, "TITLE=", 6)) { // found OGG title
				t = _strdup(t+6);

				break;
			}
		}
	}

	if (t && *t) {
		if (lstrcmpi(this->m_strCurTitle, t)) {
			// Normalize new title now
			if (this->m_strCurTitle)
				free(this->m_strCurTitle);
			this->m_strCurTitle = normalize_file_name(t);

			if (bStreamSaving && bSaveStreamsBasedOnTitle && this->m_StreamFile) {
				fclose(this->m_StreamFile);
				this->m_StreamFile = NULL;

				TCHAR szPath[MAX_PATH];
				ZeroMemory(szPath, MAX_PATH);

				lstrcpy(szPath, strStreamSavingPath);
				PathAppend(szPath, this->m_strCurTitle);
				szPath[lstrlen(szPath)] = '.';
				lstrcat(szPath, stream_type);

				if (! (this->m_StreamFile = fopen(szPath, "wb"))) {
					MessageBox(0, "Stream capture file open failed", "QMPBass Error", 0);
					bStreamSaving = 0;
				}
			}

			//if (bStreamTitle) {
			//	QCDCallbacks.Service(opSetTrackTitle, this->m_strCurTitle, (long)this->m_strPath, DIGITAL_STREAM_MEDIA);
			//}
		}

		if (u && *u)
			QCDCallbacks.Service(opSetBrowserUrl, u, 0, 0);
	}

	// Cleanup temps
	if (t)
		free(t);
	if (u)
		free(u);

	return;
}

void bass::set_stream_title(INT64 decoder_pos)
{
	if ((m_nTitleUpdateDelay > 0) && (decoder_pos >= m_nTitleUpdateDelay))
	{
		if (bStreamTitle) {
			QCDCallbacks.Service(opSetTrackTitle, m_strCurTitle, (long)m_strPath, DIGITAL_STREAM_MEDIA);
		}

		m_nTitleUpdateDelay = 0;
	}

	return;
}
void CALLBACK bass::stream_title_sync(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	bass* pBass = (bass*)user;

	// Keep title/files up-to-date
	pBass->update_stream_title((char *)BASS_ChannelGetTags(pBass->m_hBass, BASS_TAG_META));

	// Current pos + decoder buffer size + BASS_CONFIG_NET_BUFFER in ms
	pBass->m_nTitleUpdateDelay = pBass->get_current_time() + (1000 * uBufferLen) + 500;

	return;
}

void CALLBACK bass::stream_status_proc(const void *buffer, DWORD length, void *user)
{
	bass* pBass = (bass*)user;

	if (buffer)
	{
		// HTTP and ICY tags if length .eq. 0
		if (length == 0)
		{
			// Pass string to player
			QCDCallbacks.Service(opSetStatusMessage, (void *)buffer, TEXT_TOOLTIP, 0);
		} else {
			// saving stream media to local files
			if (bStreamSaving) {
				if (!pBass->m_StreamFile) {
					// show our stream saving bar
					if (bAutoShowStreamSavingBar) {
						bStreamSaveBarVisible = TRUE;
						ShowStreamSavingBar(bStreamSaveBarVisible);
						set_menu_state();
					}

					// finally, create our file
					if (PathFileExists(strStreamSavingPath)) {
						// But, only if we have a title now
						if (bSaveStreamsBasedOnTitle && pBass->m_strCurTitle && *(pBass->m_strCurTitle)) {
							TCHAR szPath[MAX_PATH];
							ZeroMemory(szPath, MAX_PATH);
							lstrcpy(szPath, strStreamSavingPath);
							PathAppend(szPath, pBass->m_strCurTitle);
							szPath[lstrlen(szPath)] = '.';
							lstrcat(szPath, stream_type);

							if ( (pBass->m_StreamFile = fopen(szPath, "wb")) ) {
								bStreamSaving = 1;

								fwrite(buffer, sizeof(char), length, pBass->m_StreamFile);
							} else {
								MessageBox(0, "Stream capture file open failed", "QMPBass Error", 0);
								bStreamSaving = 0;
							}
						}
					} else {
						MessageBox(0, "Stream capture path not found", "QMPBass Error", 0);
						bStreamSaving = 0;
					}

					UpdateSSBarStatus(hwndStreamSavingBar);
				} else
					fwrite(buffer, sizeof(char), length, pBass->m_StreamFile);
			} else {
				// Close any open file
				if (pBass->m_StreamFile) {
					fclose(pBass->m_StreamFile);
					pBass->m_StreamFile = NULL;
				}
			}
		}
	}

	return;
}

// First check the "frame sync" bits to check that you've got a frame header, and then check the relevant bits to get the bitrate.
// Note that the bit positions in that link are assuming a big-endian system.
DWORD bass::get_bitrate(void)
{
	if (!is_url && ChannelInfo.ctype & 0x1007 && r) { // VBR for mp1, mp2, mp3 local files
		DWORD framehead;
		QWORD fpos = BASS_StreamGetFilePosition(m_hBass, BASS_FILEPOS_DECODE) + BASS_StreamGetFilePosition(m_hBass, BASS_FILEPOS_START);
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
}

// Static function
void CALLBACK bass::rg_dsp_proc(HDSP handle, DWORD channel, void *buffer, DWORD length, void *user)
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
	pBass->rg_scale_factor = FLAC__replaygain_synthesis__compute_scale_factor(pBass->rg_peak, pBass->rg_gain, (double)nRGPreAmp, !(pBass->hard_limiter));
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

			m_hBass = BASS_StreamCreateURL(m_strPath, 0, 
										BASS_STREAM_STATUS | BASS_STREAM_BLOCK | 
										(is_decode ? BASS_STREAM_DECODE : 0) | 
										(use_32fp ? BASS_SAMPLE_FLOAT : 0), 
										stream_status_proc, this);
			if (m_hBass)
			{
				const char *tags = BASS_ChannelGetTags(m_hBass, BASS_TAG_ICY);
				if (tags == NULL)
					tags = BASS_ChannelGetTags(m_hBass, BASS_TAG_HTTP);
				if (tags) {
					for ( ; *tags; tags += (strlen(tags) + 1)) {
						if (!memcmp(tags, "icy-name:", 9))
							QCDCallbacks.Service(opSetTrackAlbum, (void *)(tags + 9), (long)m_strPath, DIGITAL_STREAM_MEDIA);// set broadcast name as album name
						if (!memcmp(tags, "icy-br:", 7))
							bitrate = atoi(tags + 7) * 1000; // get bitrate info of the broadcast
					}
				}

				BASS_ChannelGetInfo(m_hBass, &ChannelInfo);

				strcpy_s(stream_type, sizeof(stream_type), get_type());
				_strlwr_s(stream_type, strlen(stream_type) + 1);

				// Force update
				update_stream_title((char *)BASS_ChannelGetTags(m_hBass, BASS_TAG_META));
				if (bStreamTitle) {
					QCDCallbacks.Service(opSetTrackTitle, m_strCurTitle, (long)m_strPath, DIGITAL_STREAM_MEDIA);
				}
				BASS_ChannelSetSync(m_hBass, BASS_SYNC_META, 0, &stream_title_sync, this);

				init_rg(); // init replaygain

				if (!is_decode)
					BASS_ChannelSetDSP(m_hBass, &rg_dsp_proc, this, 1 ); // set our replaygain dsp for playback mode

				return 1;
			} else
				return (BASS_ErrorGetCode() == BASS_ERROR_FILEFORM) ? -1 : 0;
		} else
			return 1;
	} else { // for local file
		m_hBass = BASS_StreamCreateFile(FALSE, m_strPath, 0, 0, 
									(is_decode ? BASS_STREAM_DECODE : 0) | 
									(use_32fp ? BASS_SAMPLE_FLOAT : 0));
		if (m_hBass) { // for mp* and wav files
			size = BASS_StreamGetFilePosition(m_hBass, BASS_FILEPOS_END);
			length = (DWORD)BASS_ChannelBytes2Seconds( m_hBass, BASS_ChannelGetLength(m_hBass, BASS_POS_BYTE) );

			BASS_ChannelGetInfo(m_hBass, &ChannelInfo);

			if (fullinit) {
				bitrate = (DWORD)(size/(125*length)+0.5 )*1000; // bitrate (bps)

				init_rg(); // init replaygain

				if (!is_decode)
					BASS_ChannelSetDSP( m_hBass, &rg_dsp_proc, this, 1 ); // set our replaygain dsp for playback mode
			}

			return 1;
		} else if ( (m_hBass = BASS_MusicLoad(FALSE, m_strPath, 0, 0, 
			BASS_MUSIC_RAMPS | BASS_MUSIC_CALCLEN | 
			(is_decode ? BASS_MUSIC_DECODE : 0) | 
			(use_32fp ? BASS_SAMPLE_FLOAT : 0), 0)) ) { // for mod files
				size = BASS_ChannelGetLength(m_hBass, BASS_POS_BYTE);
				length = (DWORD)BASS_ChannelBytes2Seconds(m_hBass, size);

				BASS_ChannelGetInfo(m_hBass, &ChannelInfo);

				if (fullinit) {
					init_rg(); // init replaygain

					if (!is_decode)
						BASS_ChannelSetDSP( m_hBass, &rg_dsp_proc, this, 1 ); // set our replaygain dsp for playback mode
				}

				return 1;
		} else
			return (BASS_ErrorGetCode() == BASS_ERROR_FILEFORM) ? -1 : 0;
	}
}

//-----------------------------------------------------------------------------
int bass::get_data(void *out_buffer, int *out_size)
{
	LPBYTE p = (LPBYTE)out_buffer;
	int todo = *out_size;
	while (todo && BASS_ChannelIsActive(m_hBass)) {
		int rt = BASS_ChannelGetData(m_hBass, p, todo);
		if (rt == -1) { // Decode error
			// The channel is not playing, or is stalled. When handle is a "decoding channel", this indicates that it has reached the end.
			if (BASS_ErrorGetCode() == BASS_ERROR_ENDED)
				break;	// break while loop
			show_error("Decode error in %s line %d", __FILE__, __LINE__);
		}

		// If no data available, wait and try again
		if (rt == 0) {
			Sleep(20);
			continue;
		}

		p += rt;
		todo -= rt;
	}

	*out_size -= todo;

	if (*out_size <= 0) return -1; // means the end of the file

	return 1;
}

int bass::decode ( void *out_buffer, int *out_size )
{
	if (!is_decode) return 0;

	if (*out_size > m_nOut_size)
		if (!resize_reservoir(*out_size))
			return 0;

	if (get_data(m_lpDecodeReservoir, out_size) == -1)
		return -1;

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
		m_lpDecodeReservoir, 
		get_format() == 0x0003, // 32-bit floating-point data
		(*out_size)/(get_nch()*get_bps()/8), // # of wide samples
		get_nch(), // Channels
		get_bps(), // source_bps
		uResolution, // target_bps
		rg_scale_factor, 
		hard_limiter, 
		dither, 
		&dither_context
		);
	else
		*out_size = FLAC__replaygain_synthesis__apply_gain_normal(
		(FLAC__byte *)out_buffer, 
		m_lpDecodeReservoir, 
		get_format() == 0x0003, // WAVE_FORMAT_IEEE_FLOAT
		(*out_size)/(get_nch()*get_bps()/8), 
		get_nch(), 
		get_bps(), 
		rg_scale_factor, 
		hard_limiter
		);

	if (starttime == 0) starttime = timeGetTime(); // start timer which will be useful for MOD file

	return 1;
}

bool bass::resize_reservoir(int nNewSize)
{
	if (m_lpDecodeReservoir)
		delete [] m_lpDecodeReservoir;

	m_lpDecodeReservoir = new BYTE[nNewSize];
	if (!m_lpDecodeReservoir) {
		m_nOut_size = 0;
		show_error("Could not get reservoir memory!");
		return false;
	}
	m_nOut_size = nNewSize;
	return true;
}

//-----------------------------------------------------------------------------

bool bass::play(void)
{
	if (is_decode) return false;

	if (starttime == 0) starttime = timeGetTime();

	// Set BASS volume level from player
	set_volume(QCDCallbacks.Service(opGetVolume, NULL, 0, 0));

	return BASS_ChannelUpdate(m_hBass, 0) && BASS_ChannelPlay(m_hBass, FALSE) ? true : false;
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

bool bass::is_playing(void)
{
	if (is_decode) return false;

	if (!is_url)
		return BASS_ACTIVE_STOPPED != BASS_ChannelIsActive(m_hBass) ? true : false;
	else
		return true;
}

//-----------------------------------------------------------------------------

bool bass::set_volume(int level)
{
	if (is_decode) return false;

	BOOL ret = BASS_ChannelSetAttribute(m_hBass, BASS_ATTRIB_VOL, (float)level / 100.0f);

	return !!ret;
}

//-----------------------------------------------------------------------------

bool bass::fade_volume(int dst_volume, unsigned int elapse)
{
	BOOL ret = BASS_ChannelSlideAttribute(m_hBass, BASS_ATTRIB_VOL, (float)dst_volume / 100.0f, elapse);
	if (!ret) return false;

	while (BASS_ChannelIsSliding(m_hBass, BASS_ATTRIB_VOL)) Sleep(1);

	return true;
}

//-----------------------------------------------------------------------------

bool bass::set_eq(bool enabled, char const * bands) // default 10 bands for QCD
{
	bool ret = false;

	if (enabled) {
		int i = 0; // Moved here to preserve VC++ 6.0 compability
		for (i = 0; i < 10; i++)
            if (!eqfx[i]) eqfx[i] = BASS_ChannelSetFX(m_hBass, BASS_FX_DX8_PARAMEQ, 0);

		BASS_DX8_PARAMEQ eq[10];
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
