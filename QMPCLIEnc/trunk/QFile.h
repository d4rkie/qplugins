#pragma once

//#include "QString.h"
#include <atlstr.h>

// QFile ÃüÁîÄ¿±ê

class QFile
{
public:
	enum OpenFlags {
		modeRead		= (int) 0x0000, 
		modeWrite		= (int) 0x0001, 
		modeReadWrite	= (int) 0x0002, // Only one access permission 
		modeCreate		= (int) 0x0100, 
		modeNoTruncate	= (int) 0x0200, // 
	};

	QFile(void);
	QFile(HANDLE hFile);
//	QFile(LPCTSTR lpszFileName, UINT nOpenFlags);
	~QFile(void);
public:
	static void PASCAL Remove(LPCTSTR lpszFileName) { DeleteFile(lpszFileName); }
	static BOOL PASCAL FileExists(LPCTSTR lpszFileName);
public:
	BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags);
	BOOL Close(void);

	UINT Read(void* lpBuf, UINT nCount);
	LPTSTR ReadString(LPTSTR lpsz, UINT nMax);
	BOOL ReadString(CString & rString);
	UINT Write(const void* lpBuf, UINT nCount);
	BOOL WriteString(LPCSTR lpsz);

	BOOL Seek(LONGLONG lOff);
	LONGLONG GetCurPtr(void);
	LONGLONG GetLength(void);

	BOOL IsSeekable(void);

	operator HANDLE() const { return m_hFile; }
private:
	HANDLE m_hFile;
};
