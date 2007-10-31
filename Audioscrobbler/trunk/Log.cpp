#include "Precompiled.h"

#define _CRT_SECURE_NO_DEPRECATE
#include <stdarg.h>

#include "Log.h"
#include "Log.h"


//-----------------------------------------------------------------------------

CLog::CLog(HWND hwndOwner, LogMode mode, LPCTSTR strFilePath)
	: m_hwndOwner(hwndOwner), // m_strFilePath(strFilePath), 
	  m_logMode(mode), m_pFile(NULL)
{
	if (m_logMode & LOG_FILE) {
		if ( !(m_pFile = _tfsopen(strFilePath, _T("w"), _SH_DENYWR)) )
			MessageBox(m_hwndOwner, _T("Failed to open log file for writing"), _T("CLog"), MB_OK | MB_ICONEXCLAMATION);
	}
}

CLog::~CLog()
{
	if (m_pFile)
		fclose(m_pFile);
}

//-----------------------------------------------------------------------------

void CLog::OutputInfoA(MsgType type, const char* str, ...)
{
#ifndef _DEBUG
	if (type == E_DEBUG && m_logMode == LOG_NONE)
		return;
#endif

	const static size_t STR_CHARS = 2048;
	const static size_t STR_PRE = 16;
	
	char  strLog[STR_CHARS] = {0};
	char* pStrLog = strLog + STR_PRE;

	va_list args;
	va_start(args, str);
	int nCopiedChars = vsprintf_s(pStrLog, STR_CHARS-STR_PRE, str, args);
	va_end(args);

	switch (type)
	{
		case E_FATAL :
			MessageBoxA(m_hwndOwner, pStrLog, "QMP:Audioscrobbler error", MB_OK | MB_ICONEXCLAMATION);
			break;

		case E_DEBUG :
		{
			pStrLog[nCopiedChars] = '\n';
			pStrLog[nCopiedChars+1] = NULL;

			if (m_logMode == LOG_FILE)
			{
				fwrite(pStrLog, 1, nCopiedChars+1, m_pFile);
				#ifdef _DEBUG
					fflush(m_pFile);
				#endif
			}
			
			if (m_logMode == LOG_DBGCONSOLE)
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
	if (type == E_DEBUG && m_logMode == LOG_NONE)
		return;
#endif

	const static size_t STR_CHARS = 2048;
	const static size_t STR_PRE = 16;
	
	wchar_t strLog[STR_CHARS] = {0};
	wchar_t* pStrLog = strLog + STR_PRE;

	va_list args;
	va_start(args, str);
	int nCopiedChars = vswprintf_s(pStrLog, STR_CHARS-STR_PRE, str, args);
	va_end(args);

	switch (type)
	{
		case E_FATAL :
			MessageBoxW(m_hwndOwner, pStrLog, L"QMP:Audioscrobbler error", MB_OK | MB_ICONEXCLAMATION);
			break;

		case E_DEBUG :
		{
			pStrLog[nCopiedChars] = L'\n';
			pStrLog[nCopiedChars+1] = NULL;

			if (m_logMode == LOG_FILE)
			{
				fwprintf(m_pFile, pStrLog);
				#ifdef _DEBUG
					fflush(m_pFile);
				#endif
			}
			
			if (m_logMode == LOG_DBGCONSOLE)
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
	if (type == E_DEBUG && m_logMode == LOG_NONE)
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

			if (m_logMode == LOG_FILE)
			{
				fwrite(strLog+STR_PRE, 1, strlen(strLog+STR_PRE), m_pFile);
				#ifdef _DEBUG
					fflush(m_pFile);
				#endif
			}
			
			if (m_logMode == LOG_DBGCONSOLE)
			{
				for (int i = 0; i < 16; i++)
					strLog[i] = "Audioscrobbler:"[i];
				strLog[15] = ' ';
				// strLog[16] = NULL;

				OutputDebugStringA(strLog);
			}

			break;
		} // E_DEBUG
	}	// Switch
}