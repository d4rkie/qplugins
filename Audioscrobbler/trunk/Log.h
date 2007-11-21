#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>

//-----------------------------------------------------------------------------

#ifdef UNICODE
#define OutputInfo  OutputInfoW
#else
#define OutputInfo  OutputInfoA
#endif // !UNICODE

//-----------------------------------------------------------------------------

// Config.cpp depends on these values !
enum LogMode {
	LOG_NONE       = 0,
	LOG_DBGCONSOLE = 1,
	LOG_FILE       = 2
};

enum MsgType {
	E_FATAL = 0,
	E_DEBUG = 1
};

//-----------------------------------------------------------------------------

class CLog
{
private:
	struct IOQueueItem {
		int bUnicode;
		int nCchSize;

		union _pStrData {
			char* cStr;
			wchar_t* wStr;
		} pStrData;
	};

	struct IOThreadData {
		FILE*   pFile;
		WCHAR*  pwszFilePath;
		DWORD   nThreadId;
		HANDLE  hInitDoneEvent;
		HANDLE  hThread;
		CRITICAL_SECTION CS_IOQueue;
		std::deque<IOQueueItem*> IOQueue;
		BOOL    bIsRunning;
		LogMode logMode;
	} m_IOThreadData;

	void AddStringToIOList(LPCSTR str, size_t nCchSize);
	void AddStringToIOList(LPCWSTR str, size_t nCchSize);

	static DWORD WINAPI FileIOThreadProc(LPVOID lpParameter);	
	
	HWND m_hwndOwner;

public:
	CLog::CLog(HWND hwndOwner, HANDLE hInitDoneEvent, LogMode mode, LPCTSTR strFilePath);
	~CLog();

	void OutputInfoA(MsgType type, const char* str, ...);
	void OutputInfoW(MsgType type, const wchar_t* str, ...);
	void DirectOutputInfoA(MsgType type, const char* str);

	void Flush() { if (m_IOThreadData.logMode == LOG_FILE && m_IOThreadData.pFile) fflush(m_IOThreadData.pFile); };
};

#endif // __LOG_H
