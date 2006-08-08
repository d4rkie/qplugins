#pragma once

// QFile ÃüÁîÄ¿±ê

class QFile
{
public:
	enum OpenFlags {
		modeRead =         (int) 0x00000,
		modeWrite =        (int) 0x00001,
		modeReadWrite =    (int) 0x00002,
		shareCompat =      (int) 0x00000,
		shareExclusive =   (int) 0x00010,
		shareDenyWrite =   (int) 0x00020,
		shareDenyRead =    (int) 0x00030,
		shareDenyNone =    (int) 0x00040,
		modeNoInherit =    (int) 0x00080,
		modeCreate =       (int) 0x01000,
		modeNoTruncate =   (int) 0x02000,
		//typeText =         (int) 0x04000, // typeText and typeBinary are
		//typeBinary =       (int) 0x08000, // used in derived classes only
		osNoBuffer =       (int) 0x10000,
		osWriteThrough =   (int) 0x20000,
		osRandomAccess =   (int) 0x40000,
		osSequentialScan = (int) 0x80000,
		};

	QFile(void);
	QFile(HANDLE hFile);
//	QFile(LPCTSTR lpszFileName, UINT nOpenFlags);
	~QFile(void);
public:
	static void PASCAL Remove(LPCTSTR lpszFileName) { DeleteFile(lpszFileName); }
	static BOOL PASCAL FileExists(LPCTSTR lpszFileName);
public:
	BOOL Open(LPCSTR lpszFileName, UINT nOpenFlags, BOOL bUnicode = FALSE);
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
