#include "Precompiled.h"

#include "AudioscrobblerDLL.h"
#include "Audioscrobbler.h"

//-----------------------------------------------------------------------------

static const LPCSTR AS_HANDSHAKE_URL = "http://post.audioscrobbler.com:80/";
static const LPCSTR AS_CLIENT_ID     = "qcd";
static const LPCSTR AS_CLIENT_VER    = "2.0"; // old plug-in used 1.5 in latest version
//static const LPCSTR AS_CLIENT_ID     = "tst";
//static const LPCSTR AS_CLIENT_VER    = "1.0";
static const LPCSTR AS_PROTOCOL_VER  = "1.2";

//-----------------------------------------------------------------------------

enum AS_SENDSTATE {
	AS_HANDSHAKE,
	AS_NOWPLAYING,
	AS_SENDQUEUE
};

//-----------------------------------------------------------------------------

BOOL      g_bCanTryConnect      = TRUE;
BOOL      g_bHandShakeSucces    = FALSE;
BOOL      g_bMustHandshake      = TRUE;

BOOL      g_bIsPaused           = FALSE;

INT       g_nHardFailureCount   = 0;
UINT      g_nLastSubmitCount    = 0;
UINT_PTR  g_nReconnectTimer     = 0;

time_t    g_nSongStartTime      = 0;
time_t    g_nPausedTime         = 0;
time_t    g_nPauseTimestamp     = 0;

CURL* g_curl = NULL;
char g_szErrorBuffer[CURL_ERROR_SIZE] = {0};

std::string	g_strSessionID;
std::string g_strSubmissionURL;
std::string g_strNowPlayingURL;

std::deque<CAudioInfo*> g_AIQueue; // AudioInfoQueue

//-----------------------------------------------------------------------------

BOOL AS_Initialize();
void AS_CleanUp();

void AS_AddPendingSongToQueue();

void AS_LoadQueue();
void AS_SaveQueue();
void AS_GetCacheString(CAudioInfo* ai, std::string* str);

size_t AS_DataReceived(void* ptr, size_t size, size_t nmemb, void* stream);
void AS_Handshake();
void AS_SendNowPlaying(CAudioInfo* ai);
void AS_SendQueue();

void AS_HandleHardFailure();
void AS_TryReHandshake();
void AS_SettingsChanged();

//-----------------------------------------------------------------------------

DWORD WINAPI AS_Main(LPVOID lpParameter)
{
	MSG  msg;
	BOOL bRet;

	log->OutputInfo(E_DEBUG, _T("AS_Main : Started"));
	if (!AS_Initialize())
		return 0;

	log->OutputInfo(E_DEBUG, _T("AS_Main : Starting messagepump"));
	PeekMessage(&msg, NULL, 0, 0xFFFF, PM_NOREMOVE); // force message queue to be created
	Sleep(0);

	AS_LoadQueue();
	if (g_AIQueue.size() > 0)
		AS_SendQueue();

	while( (bRet = GetMessage(&msg, NULL, 0, 0)) != 0 )
	{
		if (bRet == -1) { // handle the error
			log->OutputInfo(E_DEBUG, _T("AS_Main : GetMessage return error"));
			break;
		}
		else
		{
			switch (msg.message)
			{
			case AS_MSG_PLAY_STARTED :
			{
				g_bIsPaused = FALSE;
				g_nPausedTime = 0;

				EnterCriticalSection(&g_csAIPending);
				if (g_pAIPending)
				{
					CAudioInfo ai(g_pAIPending);
					LeaveCriticalSection(&g_csAIPending);

					// Might take a long time to complete, so make sure to hold no objects
					AS_SendNowPlaying(&ai);
					
					// Update song start time, but don't submit songs shorter than 30 sec.
					EnterCriticalSection(&g_csAIPending);
					if (!g_pAIPending)
						log->OutputInfo(E_DEBUG, _T("g_AIPending is null in AS_MSG_PLAY_STARTED.\nPlease report to the author"));

					INT nTrackLength = g_pAIPending->GetTrackLength();
					if (nTrackLength < 30) {
						delete g_pAIPending;
						g_pAIPending = NULL;
						g_nSongStartTime = 0;
					}
					else
						time(&g_nSongStartTime);
					LeaveCriticalSection(&g_csAIPending);
				}
				else
					LeaveCriticalSection(&g_csAIPending);

				break;
			}

			case AS_MSG_PLAY_DONE :
			case AS_MSG_TRACK_CHANGED :
			case AS_MSG_PLAY_STOPPED :
			{
				log->OutputInfo(E_DEBUG, _T("PLAY_DONE/STOPPED: Start"));

				AS_AddPendingSongToQueue();

				log->OutputInfo(E_DEBUG, _T("PLAY_DONE/STOPPED: Queue size = %d"), g_AIQueue.size());
				if (g_AIQueue.size() > 0)
					AS_SendQueue();
			
				break;
			}

			case AS_MSG_PLAY_PAUSED :
				if (g_bIsPaused)
				{
					g_bIsPaused = FALSE; // resumed
					time_t nTime = 0;
					time(&nTime);
					g_nPausedTime = nTime - g_nPauseTimestamp;
					if (g_nPausedTime < 0)
						g_nPausedTime = 0;
				}
				else
				{
					g_bIsPaused = TRUE;
					time(&g_nPauseTimestamp);
				}
				break;

			case AS_MSG_SETTINGS_CHANGED :
			{
				log->OutputInfo(E_DEBUG, _T("AS_Main : Settings changed message"));
				g_bCanTryConnect = TRUE;
				g_bMustHandshake = TRUE;
				break;
			}

			default :
			{
				log->OutputInfo(E_DEBUG, _T("AS_Main : Unknown message"));
				break;
			}

			} // switch (msg.message)
		}
	} // while

	log->OutputInfo(E_DEBUG, _T("AS_Main : Ending"));
	AS_CleanUp();

	AS_AddPendingSongToQueue();
	AS_SaveQueue();

	// Clean AIQueue
	for (std::deque<CAudioInfo*>::iterator aiIter = g_AIQueue.begin(); aiIter != g_AIQueue.end( ); aiIter++)
		delete *aiIter;

	SetEvent(g_hASThreadEndedEvent);

	return 0;
}

//-----------------------------------------------------------------------------

BOOL AS_Initialize()
{
	curl_global_init(CURL_GLOBAL_WIN32);

	g_curl = curl_easy_init();
	if(!g_curl) {
		log->OutputInfo(E_FATAL, _T("AS_Initialize: Failed to initialize CURLLib"));
		return FALSE;
	}
	curl_easy_setopt(g_curl, CURLOPT_ERRORBUFFER, &g_szErrorBuffer);
	curl_easy_setopt(g_curl, CURLOPT_DNS_CACHE_TIMEOUT, -1);
	curl_easy_setopt(g_curl, CURLOPT_CONNECTTIMEOUT, 30);
	curl_easy_setopt(g_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(g_curl, CURLOPT_WRITEFUNCTION, &AS_DataReceived);

	g_bCanTryConnect = TRUE;
	g_nHardFailureCount = 0;
	
	return TRUE;
}

//-----------------------------------------------------------------------------

void AS_CleanUp()
{
	curl_easy_cleanup(g_curl);
	curl_global_cleanup();
}

//-----------------------------------------------------------------------------

void AS_LoadQueue()
{
	log->OutputInfo(E_DEBUG, _T("AS_LoadQueue : Start"));

	WCHAR szCacheFile[MAX_PATH] = {0};
	QMPCallbacks.Service(opGetSettingsFolder, szCacheFile, MAX_PATH*sizeof(WCHAR), 0);
	wcscat_s(szCacheFile, MAX_PATH, L"\\Audioscrobbler.cache");
	QString strCacheFile = szCacheFile;

	TiXmlDocument xmlDocCache(strCacheFile.GetMultiByte());
	if (!xmlDocCache.LoadFile())
		log->OutputInfoA(E_DEBUG, "AS_LoadQueue : Failed to read cache file - %s", xmlDocCache.ErrorDesc());
	else
	{
		TiXmlBase::SetCondenseWhiteSpace(false);
		TiXmlHandle hCache(&xmlDocCache);
		TiXmlHandle hRoot = hCache.FirstChild("Audioscrobbler_Cache");

		if (!hRoot.ToNode())
			goto cleanup;
		
		// Check version of cache document
		TiXmlElement* pVersion = hRoot.FirstChild("Version").ToElement();
		if (pVersion) {
			if (strcmp(pVersion->GetText(), "1.0") != 0) {
				log->OutputInfo(E_DEBUG, _T("AS_LoadQueue : Wrong cache version"));
				goto cleanup;
			}
		}

		CAudioInfo* ai = NULL;
		TiXmlNode* pNode = NULL;
		TiXmlElement* pEntry = hRoot.FirstChild("Entry").ToElement();
		for(pEntry; pEntry; pEntry = pEntry->NextSiblingElement())
		{
			ai = new CAudioInfo();			
			
			if (pNode = pEntry->FirstChild("Album"))
				ai->SetAlbum(pNode->ToElement()->GetText());
			if (pNode = pEntry->FirstChild("Artist"))
				ai->SetArtist(pNode->ToElement()->GetText());
			if (pNode = pEntry->FirstChild("MusicBrainId"))
				ai->SetMusicBrainTrackId(pNode->ToElement()->GetText());
			if (pNode = pEntry->FirstChild("Rating"))
				ai->SetRating(pNode->ToElement()->GetText());
			if (pNode = pEntry->FirstChild("Source"))
				ai->SetSource(pNode->ToElement()->GetText());
			if (pNode = pEntry->FirstChild("StartTime"))
				ai->SetStartTime(pNode->ToElement()->GetText());
			if (pNode = pEntry->FirstChild("Title"))
				ai->SetTitle(pNode->ToElement()->GetText());
			if (pNode = pEntry->FirstChild("TrackLength"))
				ai->SetTrackLength(pNode->ToElement()->GetText());
			if (pNode = pEntry->FirstChild("TrackNumber"))
				ai->SetTrackNumber(pNode->ToElement()->GetText());


			g_AIQueue.push_back(ai);
			log->OutputInfoA(E_DEBUG, "AS_LoadQueue : Added %s / %s to queue", ai->GetArtist(), ai->GetTitle());
		}

		// Clean the cache file
		log->OutputInfo(E_DEBUG, _T("AS_LoadQueue : Erasing cache file"));
		FILE* pFile = NULL;
		if (_tfopen_s(&pFile, strCacheFile, _T("w"))) {
			errno_t err;
			_get_errno(&err);
			log->OutputInfo(E_DEBUG, _T("AS_LoadQueue : Failed to open cache file for writing. Error nr: %d"), err);
		}
		fclose(pFile);
	}

cleanup:
	log->OutputInfo(E_DEBUG, _T("AS_LoadQueue : End"));
}

//-----------------------------------------------------------------------------

void AS_SaveQueue()
{
	log->OutputInfo(E_DEBUG, _T("AS_SaveQueue : Start"));
	
	if (g_AIQueue.size() > 0)
	{
		std::string str;
		FILE* pFile = NULL;
		
		// Create file to write
		WCHAR szCacheFile[MAX_PATH] = {0};
		QMPCallbacks.Service(opGetSettingsFolder, szCacheFile, MAX_PATH*sizeof(WCHAR), 0);
		wcscat_s(szCacheFile, MAX_PATH, L"\\Audioscrobbler.cache");

		if (_tfopen_s(&pFile, szCacheFile, _T("w"))) {
			int nErr = 0;
			WCHAR strError[256];
			_get_errno(&nErr);
			_wcserror_s(strError, 256, nErr);
			log->OutputInfo(E_DEBUG, _T("AS_SaveQueue : Failed to open cache file for writing. Error nr: %d\n\t%s\n"), nErr, strError);
			goto cleanup;
		}

		// Add header
		log->OutputInfo(E_DEBUG, _T("AS_SaveQueue : Creating XML"));
		str.append("<Audioscrobbler_Cache>\n<Version>1.0</Version>\n");
		fwrite(str.c_str(), sizeof(char), str.size(), pFile);
		str.clear();

		// Loop through the queue and add songs
		for (UINT i = 0; i < g_AIQueue.size(); i++)
		{
			str = "\t<Entry>\n";
			AS_GetCacheString(g_AIQueue.at(i), &str);
			str += "\t</Entry>\n";

			fwrite(str.c_str(), 1, str.size(), pFile);
			str.clear();
		}

		str = "</Audioscrobbler_Cache>";
		fwrite(str.c_str(), 1, str.size(), pFile);


		fclose(pFile);
	}
	else
		log->OutputInfo(E_DEBUG, _T("AS_SaveQueue : Queue size = 0"));

cleanup:
	log->OutputInfo(E_DEBUG, _T("AS_SaveQueue : End"));
}

//-----------------------------------------------------------------------------

void AS_AddPendingSongToQueue()
{
	EnterCriticalSection(&g_csAIPending);
	if (g_pAIPending)
	{
		INT nTrackLength = g_pAIPending->GetTrackLength();
		if (nTrackLength <= 0)
			delete g_pAIPending; // Is set null below
		else
		{
			// Check if song should be added to queue
			time_t nTimePlayed = 0, nTime = 0;
			time(&nTime);

			if (g_bIsPaused) {
				g_bIsPaused = FALSE;
				g_nPausedTime = nTime - g_nPauseTimestamp;
				if (g_nPausedTime < 0)
					g_nPausedTime = 0;
			}

			nTimePlayed = nTime - g_nSongStartTime - g_nPausedTime;

			// Add song if 50% played or 240 sec. whichever comes first
			if (nTrackLength/2 > 240)
				nTrackLength = 240;

			if (nTimePlayed >= 240 || nTimePlayed >= nTrackLength/2) {
				log->OutputInfoA(E_DEBUG, "Added: %s / %s to queue", g_pAIPending->GetArtist(), g_pAIPending->GetTitle());
				g_AIQueue.push_back(g_pAIPending);
			}
			else
				delete g_pAIPending;
		}		
		g_pAIPending = NULL;

		log->OutputInfo(E_DEBUG, _T("Finished updating queue"));
	} // if (g_pAIPending)
	
	LeaveCriticalSection(&g_csAIPending);
}

//-----------------------------------------------------------------------------

void AS_HandleHandshakeData(std::vector<std::string>* strLines)
{
	g_bHandShakeSucces  = FALSE;
	g_bMustHandshake    = TRUE;

	// Process data
	if (strLines->at(0).compare("OK") == 0)
	{
		g_strSessionID     = strLines->at(1);
		g_strNowPlayingURL = strLines->at(2);
		g_strSubmissionURL = strLines->at(3);
		g_nHardFailureCount = 0;
		g_bHandShakeSucces  = TRUE;
		g_bMustHandshake    = FALSE;
		g_bCanTryConnect    = TRUE;
		log->OutputInfoA(E_DEBUG, "Handshake OK\nSessionID: %s\nSubmission URL: %s\nNow playing URL: %s", g_strSessionID.c_str(), g_strSubmissionURL.c_str(), g_strNowPlayingURL.c_str());
	}
	else if (strLines->at(0).compare("BANNED") == 0)
	{
		// This indicates that this client version has been banned from the server. This usually happens if the client is violating the protocol in a destructive way. Users should be asked to upgrade their client application.
		log->OutputInfo(E_FATAL, _T("This client is banned from the Audioscrobbler network. Please upgrade!"));
		g_bCanTryConnect = FALSE;
	}
	else if (strLines->at(0).compare("BADAUTH") == 0)
	{
		// This indicates that the authentication details provided were incorrect. The client should not retry the handshake until the user has changed their details.
		log->OutputInfo(E_FATAL, _T("Username or password is not correct.\nPlease input correct details in the configuration dialog before trying to submit again."));
		g_bCanTryConnect = FALSE;
	}
	else if (strLines->at(0).compare("BADTIME") == 0)
	{
		// The timestamp provided was not close enough to the current time. The system clock must be corrected before re-handshaking.
		log->OutputInfo(E_FATAL, _T("Failed on handshake with Audioscrobbler server.\n\nThe system clock is not correct.\nPlease correct and restart the player"));
		g_bCanTryConnect = FALSE;
	}
	else if (strLines->at(0).compare(0, 6, "FAILED") == 0)
	{
		// This indicates a temporary server failure. The reason indicates the cause of the failure. The client should proceed as directed in the Hard Failures section.
		g_bCanTryConnect = FALSE;
		AS_TryReHandshake();
	}
	else
	{
		// Hard failure
		// An error may be reported to the user, but as with other messages this should be kept to a minimum.
		// AS_HandleHardFailure();
		log->OutputInfoA(E_DEBUG, "AS_HandleHandshakeData unknown response.\nFirst line response was: %s", strLines->at(0).c_str());
	}
}

//-----------------------------------------------------------------------------

void AS_HandleNowPlayingData(std::vector<std::string>* strLines)
{
	// Process data
	if (strLines->at(0).compare("OK") == 0)
	{
		log->OutputInfo(E_DEBUG, _T("Now-Playing info set successfully"));
	}
	else if (strLines->at(0).compare("BADSESSION") == 0)
	{
		log->OutputInfo(E_DEBUG, _T("Now-Playing returned BADSESSION"));
		g_bHandShakeSucces = FALSE;
		g_bMustHandshake = TRUE;
	}
	else if (strLines->at(0).compare(0, 6, "FAILED") == 0)
	{
		log->OutputInfoA(E_DEBUG, "Now-Playing : %s", strLines->at(0).c_str());
	}
	else
	{
		log->OutputInfo(E_DEBUG, _T("Now-Playing -> Unknown server response"));
	}
}

//-----------------------------------------------------------------------------

void AS_HandleSendQueueData(std::vector<std::string>* strLines)
{
	// Process data
	if (strLines->at(0).compare("OK") == 0)
	{
		log->OutputInfo(E_DEBUG, _T("Submission accepted by server."));
		// Delete all the songs submitted from the queue
		for (UINT i = 0; i < g_nLastSubmitCount; i++)
			g_AIQueue.pop_front();
	}
	else if (strLines->at(0).compare("BADSESSION") == 0)
	{
		log->OutputInfo(E_DEBUG, _T("Submission returned BADSESSION"));
		g_bHandShakeSucces = FALSE;
		g_bMustHandshake = TRUE;
	}
	else if (strLines->at(0).compare(0, 6, "FAILED") == 0)
	{
		log->OutputInfoA(E_DEBUG, "Submission error : %s", strLines->at(0).c_str());
		AS_HandleHardFailure();
	}
	else
	{
		log->OutputInfo(E_DEBUG, _T("Submission : Unknown server response"));
		AS_HandleHardFailure();
	}
}

//-----------------------------------------------------------------------------

size_t AS_DataReceived(void* ptr, size_t size, size_t nmemb, void* stream)
{
	// Allocate buffer for the response
	char* str = new char[nmemb*size + 1];
	memcpy(str, ptr, nmemb*size);
	str[nmemb*size] = 0;

	// log->OutputInfoA(E_DEBUG, "Data received: %s", str);
	
	// Get all lines and convert to std::string
	std::vector<std::string> strLines;
	std::string strLine;
	
	char* next_token = 0;
	char* token = strtok_s(str, "\n", &next_token);
   while( token != NULL )
   {
		strLine = token;
		strLines.push_back(strLine);

      token = strtok_s(NULL, "\n", &next_token);
   }

	switch (*(int*)stream)
	{
		case AS_HANDSHAKE :
			AS_HandleHandshakeData(&strLines);
			break;
		case AS_NOWPLAYING :
			AS_HandleNowPlayingData(&strLines);
			break;
		case AS_SENDQUEUE :
			AS_HandleSendQueueData(&strLines);
			break;

		default :
			log->OutputInfo(E_DEBUG, _T("AS_DataReceived : No data handler"));
	}

	// Cleanup and return size of processed data
	delete [] str;
	// strLines.clear();
	
	return nmemb*size;
}

//-----------------------------------------------------------------------------

void AS_Handshake()
{
	// http://post.audioscrobbler.com/
	// ?hs=true&p=1.2&c=<client-id>&v=<client-ver>&u=<user>&t=<timestamp>&a=<auth>
	// token := md5(md5(password) + timestamp)
	log->OutputInfo(E_DEBUG, _T("AS_Handshake: Starting..."));

	if (!g_bCanTryConnect) {
		log->OutputInfo(E_DEBUG, _T("g_bCanTryConnect false. Exiting"));
		return;
	}
	if (g_bIsClosing)
		return;

	char strTime[16] = {0};
	char strTmp[64] = {0};
	char strLongAuth[64] = {0};
	char strBuffer[512] = {0};

	g_bHandShakeSucces = FALSE;

	// Get unix timestamp and convert to string
	time_t ltime;
	time(&ltime);
	_ui64toa_s(ltime, strTime, sizeof(strTime), 10);
	
	// Create auth token
	for (int i = 0; i < 32; i++)
		strTmp[i] = Settings.strPassword[i];
	for (int i = 0; strTime[i]; i++)
		strTmp[i+32] = strTime[i];
	md5_32(strLongAuth, (const BYTE*)strTmp);
	strLongAuth[32] = 0;
	
	// Create the post string
	int nChars = _snprintf_s(strBuffer, sizeof(strBuffer), sizeof(strBuffer)-1,
		"%s?hs=true&p=%s&c=%s&v=%s&u=%s&t=%s&a=%s",
		AS_HANDSHAKE_URL, AS_PROTOCOL_VER, AS_CLIENT_ID, AS_CLIENT_VER, Settings.strUsername.GetMultiByte(), strTime, strLongAuth);

	if (nChars < 10) {
		log->OutputInfo(E_DEBUG, _T("Failed to make handshake string"));
		return;
	}

	log->OutputInfoA(E_DEBUG, strBuffer);

	// Set curl options
	int nOperation = AS_HANDSHAKE;
	curl_easy_setopt(g_curl, CURLOPT_WRITEDATA, &nOperation);
	curl_easy_setopt(g_curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(g_curl, CURLOPT_URL, strBuffer);

	CURLcode nResult = curl_easy_perform(g_curl);
	
	if (nResult == 6) {
		// Could not resolve host - Happens first time, so we retry
		log->OutputInfo(E_DEBUG, _T("AS_Handshake: Retrying to handshake once"));
		nResult = curl_easy_perform(g_curl);
	}
	
	if (nResult != 0)
		log->OutputInfoA(E_DEBUG, "AS_Handshake: Perform error - Code: %d\nError buffer: %s\n", nResult, g_szErrorBuffer);
	else
		log->OutputInfo(E_DEBUG, _T("AS_Handshake: Perform success"));
}

//-----------------------------------------------------------------------------

void AS_SendNowPlaying(CAudioInfo* ai)
{
	log->OutputInfoA(E_DEBUG, "AS_SendNowPlaying: Now-Playing: %s / %s", ai->GetArtist(), ai->GetTitle());

	if (!g_bCanTryConnect) {
		log->OutputInfo(E_DEBUG, _T("g_bCanTryConnect false. Exiting"));
		return;
	}
	if (g_bMustHandshake)
		AS_Handshake();
	if (!g_bHandShakeSucces)
		return;
	if (g_bIsClosing)
		return;

	// The submission takes place as a HTTP/1.1 POST transaction with the server,
	// using the URL provided during the handshake phase of the protocol.
	// The values given to the keys must be converted to UTF-8 first, 
	// and must be URL encoded. 
	// ! == required
	// * == empty if unknown
	// ?s=!<sessionID>&a=!<artist>&t=!<track>b=*<album>&l=*<secs>&n=*<tracknumber>&m=*<mb-trackid>

	// Create the post string
	char strBuffer[2048] = {0};
	
	char* szArtist = curl_easy_escape(g_curl, ai->GetArtist(), 0);
	char* szTitle  = curl_easy_escape(g_curl, ai->GetTitle(), 0);
	char* szAlbum  = curl_easy_escape(g_curl, ai->GetAlbum(), 0);

	int nPostFieldSize = _snprintf_s(strBuffer, sizeof(strBuffer), sizeof(strBuffer)-1, 
		"s=%s&a=%s&t=%s&b=%s&l=%d&n=%d&m=%s",
		g_strSessionID.c_str(), szArtist, szTitle, szAlbum, 
		ai->GetTrackLength(), ai->GetTrackNumber(), ai->GetMusicBrainTrackId()
	);

	log->DirectOutputInfoA(E_DEBUG, strBuffer);
	
	curl_free(szArtist);
	curl_free(szTitle);
	curl_free(szAlbum);

	
	// Set options
	int nOperation = AS_NOWPLAYING;
	curl_easy_setopt(g_curl, CURLOPT_WRITEDATA, &nOperation);
	curl_easy_setopt(g_curl, CURLOPT_POST, 1);
	curl_easy_setopt(g_curl, CURLOPT_POSTFIELDS, strBuffer);
	curl_easy_setopt(g_curl, CURLOPT_POSTFIELDSIZE, nPostFieldSize); 
	curl_easy_setopt(g_curl, CURLOPT_URL, g_strNowPlayingURL.c_str());
	

	// Perform the request
	CURLcode nResult = curl_easy_perform(g_curl);
	if (nResult != 0) {
		log->OutputInfoA(E_DEBUG, "AS_SendNowPlaying: Perform error - Code: %d\nError buffer: %s\n", nResult, g_szErrorBuffer);
		AS_HandleHardFailure();
	}
	else
		log->OutputInfo(E_DEBUG, _T("AS_SendNowPlaying: Perform success\n"));
}

//-----------------------------------------------------------------------------

void AS_SendQueue()
{
	log->OutputInfo(E_DEBUG, _T("AS_SendQueue: Starting to send cached information"));
	if (g_AIQueue.empty()) {
		log->OutputInfo(E_DEBUG, _T("Queue is empty"));
		return;
	}

	if (!g_bCanTryConnect) {
		log->OutputInfo(E_DEBUG, _T("g_bCanTryConnect false. Exiting"));
		return;
	}
	if (g_bMustHandshake)
		AS_Handshake();
	if (!g_bHandShakeSucces)
		return;
	if (g_bIsClosing)
		return;

	// Create song submit list
	log->DirectOutputInfoA(E_DEBUG, "AS_SendQueue: Building queue information:");
	const static int CACHE_SIZE = 2048;
	char strI[4] = {0};
	char* strCache = new char[CACHE_SIZE];
	std::string strSongs;

	strSongs.assign("s=").append(g_strSessionID);
	log->DirectOutputInfoA(E_DEBUG, strSongs.c_str());

	for (g_nLastSubmitCount = 0; g_nLastSubmitCount < g_AIQueue.size() && g_nLastSubmitCount < 50; g_nLastSubmitCount++)
	{
		strI[0] = NULL;
		ZeroMemory(strCache, CACHE_SIZE);		
		CAudioInfo* ai = g_AIQueue.at(g_nLastSubmitCount);
		
		_itoa_s(g_nLastSubmitCount, strI, sizeof(strI), 10);
		char* szArtist = curl_easy_escape(g_curl, ai->GetArtist(), 0);
		char* szTitle  = curl_easy_escape(g_curl, ai->GetTitle(), 0);
		char* szAlbum  = curl_easy_escape(g_curl, ai->GetAlbum(), 0);

		// Insert indiciers
		_snprintf_s(strCache, 2048, 2048, 
			"&a[%s]=%s&t[%s]=%s&i[%s]=%s&o[%s]=%s&r[%s]=&l[%s]=%d&b[%s]=%s&n[%s]=%d&m[%s]=%s",
			strI, szArtist, // a
			strI, szTitle,  // t
			strI, ai->GetStartTime(), // i
			strI, "U",      // o
			strI,           // r
			strI, ai->GetTrackLength(), 
			strI, szAlbum, 
			strI, ai->GetTrackNumber(),
			strI, ai->GetMusicBrainTrackId()
		);

		curl_free(szArtist);
		curl_free(szTitle);
		curl_free(szAlbum);

		strSongs.append(strCache);
		log->DirectOutputInfoA(E_DEBUG, strCache);
	}

	delete [] strCache;

	
	// Set curl options
	int nOperation = AS_SENDQUEUE;
	curl_easy_setopt(g_curl, CURLOPT_WRITEDATA, &nOperation);
	curl_easy_setopt(g_curl, CURLOPT_POST, 1);
	curl_easy_setopt(g_curl, CURLOPT_POSTFIELDS, strSongs.c_str());
	curl_easy_setopt(g_curl, CURLOPT_POSTFIELDSIZE, strSongs.size()); 
	curl_easy_setopt(g_curl, CURLOPT_URL, g_strSubmissionURL.c_str());


	// Perform the request
	CURLcode nResult = curl_easy_perform(g_curl);
	if (nResult != 0) {
		log->OutputInfoA(E_DEBUG, "AS_SendQueue: Perform error - Code: %d\nError buffer: %s\n", nResult, g_szErrorBuffer);
		AS_HandleHardFailure();
	}
	else {
		log->OutputInfo(E_DEBUG, _T("AS_SendQueue: Perform success"));
	}
}

//-----------------------------------------------------------------------------

void AS_HandleHardFailure()
{
	g_nHardFailureCount++;

	if (g_nHardFailureCount >= 3) {
		log->OutputInfo(E_DEBUG, _T("3 hard failures. Client must handshake again!"));
		
		g_bMustHandshake = TRUE;
		g_bCanTryConnect = TRUE;
		g_strSessionID = "";
		g_strSubmissionURL = "";
		g_strNowPlayingURL = "";
	}
}

//-----------------------------------------------------------------------------

void CALLBACK AS_cbSendQueue(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
	if (g_nReconnectTimer) {
		KillTimer(NULL, g_nReconnectTimer);
		g_nReconnectTimer = 0;
	}

	g_bCanTryConnect = TRUE;
	g_bMustHandshake = TRUE;
	g_bHandShakeSucces = FALSE;
	
	AS_SendQueue();
}

//-----------------------------------------------------------------------------

void AS_TryReHandshake()
{
	static int nMinutes = 1;

	g_nReconnectTimer = SetTimer(NULL, NULL, nMinutes * 60 * 1000, AS_cbSendQueue);
	
	// Set minutes
	if (nMinutes >= 120)
		nMinutes = 120;
	else
		nMinutes = nMinutes * 2;
}

//-----------------------------------------------------------------------------

void AS_GetCacheString(CAudioInfo* ai, std::string* str)
{
	// CAudioInfo data is UTF8, so we can safely just write it

	char strTrackNumber[32] = {0};
	char strTrackLength[32] = {0};
	_itoa_s(ai->GetTrackNumber(), strTrackNumber, sizeof(strTrackNumber), 10);
	_itoa_s(ai->GetTrackLength(), strTrackLength, sizeof(strTrackLength), 10);

	str->append("\t\t<Album>").append(ai->GetAlbum()).append("</Album>\n");
	str->append("\t\t<Artist>").append(ai->GetArtist()).append("</Artist>\n");
	str->append("\t\t<MusicBrainId>").append(ai->GetMusicBrainTrackId()).append("</MusicBrainId>\n");
	str->append("\t\t<Rating>").append(ai->GetRating()).append("</Rating>\n");
	str->append("\t\t<Source>").append(ai->GetSource()).append("</Source>\n");
	str->append("\t\t<StartTime>").append(ai->GetStartTime()).append("</StartTime>\n");
	str->append("\t\t<Title>").append(ai->GetTitle()).append("</Title>\n");
	str->append("\t\t<TrackLength>").append(strTrackLength).append("</TrackLength>\n");	
	str->append("\t\t<TrackNumber>").append(strTrackNumber).append("</TrackNumber>\n");
}





#if 0
	for (std::deque<CAudioInfo*>::iterator aiIter = g_AIQueue.begin(); aiIter != g_AIQueue.end( ); aiIter++)
	{
		CAudioInfo* ai = *aiIter;
		log->OutputInfo(E_DEBUG, _T("%s : %s"), ai->GetArtist(), ai->GetTitle());
	}
#endif

/*
When to Submit

The client should monitor the user's interaction with the music playing service to
whatever extent the service allows. In order to qualify for submission all of the
following criteria must be met:

   1. The track must be submitted once it has finished playing.
		Whether it has finished playing naturally or has been manually stopped by the user is irrelevant.
   2. The track must have been played for a duration of at least 240 seconds or half the track's 
		total length, whichever comes first. Skipping or pausing the track is irrelevant as long as 
		the appropriate amount has been played.
   3. The total playback time for the track must be more than 30 seconds. Do not submit tracks shorter than this.
   4. Unless the client has been specially configured, it should not attempt to interpret filename 
		information to obtain metadata instead of tags (ID3, etc).
*/