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
	FILE* m_pFile;
	//LPCTSTR m_strFilePath;
	HWND m_hwndOwner;
	LogMode m_logMode;

public:
	CLog::CLog(HWND hwndOwner, LogMode mode, LPCTSTR strFilePath);
	~CLog();

	void OutputInfoA(MsgType type, const char* str, ...);
	void OutputInfoW(MsgType type, const wchar_t* str, ...);
	void DirectOutputInfoA(MsgType type, const char* str);
};

#endif // __LOG_H
