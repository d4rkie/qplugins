#include "Precompiled.h"

#define _CRT_SECURE_NO_DEPRECATE
#include <stdarg.h>

#include "Log.h"
#include "Log.h"

static const int IOTHREAD_SLEEP_MS = 50;

//-----------------------------------------------------------------------------
// hInitDoneEvent handle is closed by caller

CLog::CLog(HWND hwndOwner, HANDLE hInitDoneEvent, LogMode mode, LPCTSTR ptszFilePath)
	: m_hwndOwner(hwndOwner)
{
	m_IOThreadData.bIsRunning     = TRUE;
	m_IOThreadData.logMode        = mode;
	m_IOThreadData.nThreadId      = 0;
	m_IOThreadData.hThread        = NULL;
	m_IOThreadData.pFile          = NULL;
	m_IOThreadData.hInitDoneEvent = hInitDoneEvent;

	size_t nLen = _tcslen(ptszFilePath);
	m_IOThreadData.pwszFilePath = new WCHAR[nLen + 1];
	#ifdef UNICODE
		wcscpy_s(m_IOThreadData.pwszFilePath, nLen+1, ptszFilePath);
	#else
		MultiByteToWideChar(CP_ACP, NULL, ptszFilePath, nLen, m_IOThreadData.pwszFilePath, nLen+1); 
	#endif

	if (m_IOThreadData.logMode & LOG_FILE) {
		m_IOThreadData.hThread = CreateThread(NULL, 0, CLog::FileIOThreadProc, &m_IOThreadData, 0, &(m_IOThreadData.nThreadId));
		SetThreadPriority(m_IOThreadData.hThread, THREAD_PRIORITY_BELOW_NORMAL);
	}
	else
		SetEvent(hInitDoneEvent);
}

//-----------------------------------------------------------------------------

CLog::~CLog()
{
	// Close IO thread
	if (m_IOThreadData.hThread)
	{
		m_IOThreadData.bIsRunning = FALSE;
		WaitForSingleObject(m_IOThreadData.hThread, INFINITE);
		CloseHandle(m_IOThreadData.hThread);
		m_IOThreadData.hThread = INVALID_HANDLE_VALUE;	
	}
}

//-----------------------------------------------------------------------------

DWORD WINAPI CLog::FileIOThreadProc(LPVOID lpParameter)
{
	DWORD nReturn = 0;
	char szBuf[256] = {0};
	IOQueueItem* qi = NULL;

	IOThreadData* iotd = reinterpret_cast<IOThreadData*>(lpParameter);

	iotd->pFile = _wfsopen(iotd->pwszFilePath, L"w", _SH_DENYWR);

	if ( !(iotd->pFile) )
	{
		iotd->logMode = LOG_NONE;
		MessageBox(NULL, _T("Failed to open log file for writing"), _T("CLog"), MB_OK | MB_ICONEXCLAMATION);
	}
	else
	{
		char szTimeBuf[16] = {0};
		__time64_t nNow = 0;
		tm now;

		_tzset();
		_time64(&nNow);
		_gmtime64_s( &now, &nNow );

		strftime(szBuf, sizeof(szBuf), "Audioscrobbler log file\nDate created: %d. %b %y\n\n", &now);			

		fwrite(szBuf, sizeof(char), strlen(szBuf), iotd->pFile);
		fflush(iotd->pFile);
	}

	// We don't need the filepath any more
	delete [] iotd->pwszFilePath;

	// Init critical section
	InitializeCriticalSection(&(iotd->CS_IOQueue));

	// We are done initializing, so tell caller the we are ready to be used
	SetEvent(iotd->hInitDoneEvent);

	while (iotd->bIsRunning)
	{
		// It should be safe to check for empty without lock, as long as we are the only consumer
		if (!iotd->IOQueue.empty())
		{
			// Lock IOQueue, remove first QueueItem, release lock
			// EnterCriticalSection should be cheap as long as we were the last to hold it
			EnterCriticalSection(&(iotd->CS_IOQueue));
			qi = iotd->IOQueue.front();
			iotd->IOQueue.pop_front();
			LeaveCriticalSection(&(iotd->CS_IOQueue));

			if (qi->bUnicode)
			{
				qi->pStrData.wStr[qi->nCchSize-1] = NULL;
				fwprintf(iotd->pFile, qi->pStrData.wStr);
				delete [] qi->pStrData.wStr;
			}
			else
			{
				fwrite(qi->pStrData.cStr, sizeof(char), qi->nCchSize, iotd->pFile);
				delete [] qi->pStrData.cStr;
			}

			delete qi;

			#ifdef _DEBUG
				fflush(iotd->pFile);
			#endif
		}
		else
		{
			// We only sleep if queue is empty.
			Sleep(IOTHREAD_SLEEP_MS);
		}
	}

	DeleteCriticalSection(&(iotd->CS_IOQueue));

	// Write out last messages
	while (!iotd->IOQueue.empty())
	{
		qi = iotd->IOQueue.front();
		iotd->IOQueue.pop_front();
		if (qi->bUnicode)
		{
			qi->pStrData.wStr[qi->nCchSize-1] = NULL;
			fwprintf(iotd->pFile, qi->pStrData.wStr);
			delete [] qi->pStrData.wStr;
		}
		else
		{
			fwrite(qi->pStrData.cStr, sizeof(char), qi->nCchSize, iotd->pFile);
			delete [] qi->pStrData.cStr;
		}

		delete qi;
	}
	
	strcpy_s(szBuf, sizeof(szBuf), "\nAudioscrobbler log file thread ending");
	fwrite(szBuf, sizeof(char), strlen(szBuf), iotd->pFile);
	fflush(iotd->pFile);

	fclose(iotd->pFile);
	iotd->pFile = NULL;

	return nReturn;
}

//-----------------------------------------------------------------------------

void CLog::AddStringToIOList(LPCSTR str, size_t nCchSize)
{
	const int nNewStrSize = nCchSize + 9 + 1; // nStrSize + null + "00:00:00 "
	LPSTR pNewStr = new char[nNewStrSize];

	// Append timestamp
	_strtime_s(pNewStr, nNewStrSize);
	pNewStr[8] = ' '; pNewStr[9] = NULL; pNewStr[10] = NULL;
	
	strcat_s(pNewStr, nNewStrSize, str);

	IOQueueItem* qi = new IOQueueItem;
	qi->bUnicode = FALSE;
	qi->nCchSize = nNewStrSize-1;
	qi->pStrData.cStr = pNewStr;

	EnterCriticalSection(&(m_IOThreadData.CS_IOQueue));
	m_IOThreadData.IOQueue.push_back(qi);
	LeaveCriticalSection(&(m_IOThreadData.CS_IOQueue));
}

//-----------------------------------------------------------------------------

void CLog::AddStringToIOList(LPCWSTR str, size_t nCchSize)
{
	const int nNewStrSize = nCchSize + 9 + 1; // nStrSize + null + "00:00:00 "
	LPWSTR pNewStr = new WCHAR[nNewStrSize];

	// Append timestamp
	_wstrtime_s(pNewStr, nNewStrSize);
	pNewStr[8] = ' '; pNewStr[9] = NULL; pNewStr[10] = NULL;
	
	wcscat_s(pNewStr, nNewStrSize, str);

	IOQueueItem* qi = new IOQueueItem;
	qi->bUnicode = TRUE;
	qi->nCchSize = nNewStrSize;
	qi->pStrData.wStr = pNewStr;

	EnterCriticalSection(&(m_IOThreadData.CS_IOQueue));
	m_IOThreadData.IOQueue.push_back(qi);
	LeaveCriticalSection(&(m_IOThreadData.CS_IOQueue));
}

//-----------------------------------------------------------------------------

void CLog::OutputInfoA(MsgType type, const char* str, ...)
{
#ifndef _DEBUG
	if (type == E_DEBUG && m_IOThreadData.logMode == LOG_NONE)
		return;
#endif

	const static size_t STR_CHARS = 2048;
	const static size_t STR_PRE = 16;
	
	char  strLog[STR_CHARS] = {0};
	char* pStrLog = strLog + STR_PRE;

	va_list args;
	va_start(args, str);
	int nCopiedChars = vsprintf_s(pStrLog, STR_CHARS-STR_PRE-1, str, args);
	va_end(args);

	switch (type)
	{
		case E_FATAL :
			MessageBoxA(m_hwndOwner, pStrLog, "QMP:Audioscrobbler error", MB_OK | MB_ICONEXCLAMATION);
			break;

		case E_DEBUG :
		{
			pStrLog[nCopiedChars]   = '\n';
			pStrLog[nCopiedChars+1] = NULL;

			if (m_IOThreadData.logMode == LOG_FILE)
			{
				AddStringToIOList( pStrLog, nCopiedChars+1 );
			}
			else if (m_IOThreadData.logMode == LOG_DBGCONSOLE)
			{
				for (int i = 0; i < 16; i++)
					strLog[i] = "Audioscrobbler:"[i];
				strLog[15] = ' ';
			
				OutputDebugStringA(strLog);
			}

			break;
		} // E_DEBUG
	}	// Switch
}

void CLog::OutputInfoW(MsgType type, const wchar_t* str, ...)
{
#ifndef _DEBUG
	if (type == E_DEBUG && m_IOThreadData.logMode == LOG_NONE)
		return;
#endif

	const static size_t STR_CHARS = 2048;
	const static size_t STR_PRE = 16;
	
	wchar_t  strLog[STR_CHARS] = {0};
	wchar_t* pStrLog = strLog + STR_PRE;

	va_list args;
	va_start(args, str);
	int nCopiedChars = vswprintf_s(pStrLog, STR_CHARS-STR_PRE-1, str, args);
	va_end(args);

	if (nCopiedChars == -1) {
		MessageBoxW(m_hwndOwner, L"Failed to write log entry", L"QMP:Audioscrobbler error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	switch (type)
	{
		case E_FATAL :
			MessageBoxW(m_hwndOwner, pStrLog, L"QMP:Audioscrobbler error", MB_OK | MB_ICONEXCLAMATION);
			break;

		case E_DEBUG :
		{
			pStrLog[nCopiedChars]   = L'\n';
			pStrLog[nCopiedChars+1] = NULL;

			if (m_IOThreadData.logMode == LOG_FILE)
			{
				AddStringToIOList( pStrLog, nCopiedChars+1 );
			}
			else if (m_IOThreadData.logMode == LOG_DBGCONSOLE)
			{
				for (int i = 0; i < 16; i++)
					strLog[i] = L"Audioscrobbler:"[i];
				strLog[15] = L' ';
			
				OutputDebugStringW(strLog);
			}

			break;
		} // E_DEBUG
	}	// Switch
}

//-----------------------------------------------------------------------------
// DirectOutput
// Used for output that has a chance to break sprintf. F.x. url encoded strings
//-----------------------------------------------------------------------------

void CLog::DirectOutputInfoA(MsgType type, const char* str)
{
#ifndef _DEBUG
	if (type == E_DEBUG && m_IOThreadData.logMode == LOG_NONE)
		return;
#endif
	
	switch (type)
	{
		case E_FATAL :
			MessageBoxA(m_hwndOwner, str, "QMP:Audioscrobbler error", MB_OK | MB_ICONEXCLAMATION);
			break;

		case E_DEBUG :
		{
			const static size_t STR_CHARS = 2048;
			const static size_t STR_PRE = 16;

			char strLog[STR_CHARS] = {0};

			strcpy_s(strLog+STR_PRE, STR_CHARS-STR_PRE, str);
			strcat_s(strLog+STR_PRE, STR_CHARS-STR_PRE, "\n");

			if (m_IOThreadData.logMode == LOG_FILE)
			{
				strLog[STR_CHARS - 1] = NULL;
				AddStringToIOList( strLog+STR_PRE, strlen(strLog+STR_PRE) );
			}
			else if (m_IOThreadData.logMode == LOG_DBGCONSOLE)
			{
				for (int i = 0; i < 16; i++)
					strLog[i] = "Audioscrobbler:"[i];
				strLog[15] = ' ';

				OutputDebugStringA(strLog);
			}

			break;
		} // E_DEBUG
	}	// Switch
}