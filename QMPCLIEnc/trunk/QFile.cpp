// QFile.cpp : 实现文件
//

#include "stdafx.h"

#include ".\qfile.h"


// QFile

QFile::QFile(void)
: m_hFile(INVALID_HANDLE_VALUE)
{
}

QFile::QFile(HANDLE hFile)
{
	m_hFile = hFile;
}
/*
QFile::QFile(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	ASSERT(AfxIsValidString(lpszFileName));
	
	m_hFile = (UINT_PTR) hFileNull;
	CFileException e;
	if (!Open(lpszFileName, nOpenFlags, &e))
		AfxThrowFileException(e.m_cause, e.m_lOsError, e.m_strFileName);
}
*/
QFile::~QFile(void)
{
	Close();
}

// QFile 成员函数

BOOL QFile::Open(LPCSTR lpszFileName, UINT nOpenFlags, BOOL bUnicode)
{
	if ( Close()) {
		// map read/write mode
		DWORD dwAccess = 0;
		switch (nOpenFlags & 3)
		{
		case modeRead:
			dwAccess = GENERIC_READ;
			break;
		case modeWrite:
			dwAccess = GENERIC_WRITE;
			break;
		case modeReadWrite:
			dwAccess = GENERIC_READ|GENERIC_WRITE;
			break;
		default:
			return FALSE; // invalid share mode
		}

		// map share mode
		DWORD dwShareMode = 0;
		switch (nOpenFlags & 0x70)    // map compatibility mode to exclusive
		{
		default:
			return FALSE;  // invalid share mode?
		case shareCompat:
		case shareExclusive:
			dwShareMode = 0;
			break;
		case shareDenyWrite:
			dwShareMode = FILE_SHARE_READ;
			break;
		case shareDenyRead:
			dwShareMode = FILE_SHARE_WRITE;
			break;
		case shareDenyNone:
			dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
			break;
		}

		// map modeNoInherit flag
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = (nOpenFlags & modeNoInherit) == 0;

		// map creation flags
		DWORD dwCreateFlag;
		if ( nOpenFlags & modeCreate) {
			if (nOpenFlags & modeNoTruncate)
				dwCreateFlag = OPEN_ALWAYS;
			else
				dwCreateFlag = CREATE_ALWAYS;
		}
		else
			dwCreateFlag = OPEN_EXISTING;

		// special system-level access flags

		// Random access and sequential scan should be mutually exclusive

		DWORD dwFlags = FILE_ATTRIBUTE_NORMAL;
		if (nOpenFlags & osNoBuffer)
			dwFlags |= FILE_FLAG_NO_BUFFERING;
		if (nOpenFlags & osWriteThrough)
			dwFlags |= FILE_FLAG_WRITE_THROUGH;
		if (nOpenFlags & osRandomAccess)
			dwFlags |= FILE_FLAG_RANDOM_ACCESS;
		if (nOpenFlags & osSequentialScan)
			dwFlags |= FILE_FLAG_SEQUENTIAL_SCAN;

		// attempt file creation
		if ( bUnicode)
			m_hFile = ::CreateFileW( (LPCWSTR)lpszFileName, 
			dwAccess, dwShareMode, &sa, dwCreateFlag, dwFlags, 
			NULL);
		else
			m_hFile = ::CreateFile( lpszFileName, 
			dwAccess, dwShareMode, &sa, dwCreateFlag, dwFlags, 
			NULL);
		if ( m_hFile == INVALID_HANDLE_VALUE)
			return FALSE;
		else
			return TRUE;
	} else {
		return FALSE;
	}
}

BOOL QFile::Close(void)
{
	if ( m_hFile != INVALID_HANDLE_VALUE) {
		if ( CloseHandle(m_hFile)) {
			m_hFile = INVALID_HANDLE_VALUE;

			return TRUE;
		} else {
			return FALSE;
		}
	} else {
		return TRUE; // always return true for closed/havn't been opened file.
	}
}

UINT QFile::Read(void* lpBuf, UINT nCount)
{
	if ( m_hFile == INVALID_HANDLE_VALUE)
		return 0;

    DWORD bytesread;
    if ( !ReadFile( m_hFile, lpBuf, nCount, &bytesread, NULL))
		return 0;
	else
		return bytesread;
}

LPTSTR QFile::ReadString(LPTSTR lpsz, UINT nMax)
{
	if ( lpsz == NULL || nMax == 0 || m_hFile == INVALID_HANDLE_VALUE)
		return NULL;

	BYTE byte_r, byte_w;

	BOOL end = FALSE;
	if ( Read(&byte_r, 1)) {
		byte_w = byte_r;
		for ( UINT n = 0; n < nMax - 1 && Read( &byte_r, 1); n++) {
			if ( byte_w == 0x0d && byte_r == 0x0a) { // OK, we've reached the end of a line
				end = TRUE;
				break;
			} else {
				lpsz[n] = byte_w;
				byte_w = byte_r;
			}
		}
		lpsz[n++] = end ? '\n' : byte_w;
		lpsz[n] = '\0';

		return lpsz;
	} else {
		return NULL;
	}
}

BOOL QFile::ReadString(CString & rString)
{
	if ( m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	rString = _T("");    // empty string without deallocating
	const int nMaxSize = 128;
	LPTSTR lpsz = rString.GetBuffer(nMaxSize);
	LPTSTR lpszResult;
	int nLen = 0;
	for (;;) {
		lpszResult = ReadString(lpsz, nMaxSize+1);
		rString.ReleaseBuffer();
/*
		// handle error/eof case
		if (lpszResult == NULL && !feof(m_pStream)) {
			clearerr(m_pStream);
			AfxThrowFileException(CFileException::generic, _doserrno, 
				m_strFileName);
		}
*/
		// if string is read completely or EOF
		if (lpszResult == NULL ||
			(nLen = (int)lstrlen(lpsz)) < nMaxSize ||
			lpsz[nLen-1] == '\n')
			break;

		nLen = rString.GetLength();
		lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;
	}

	// remove '\n' from end of string if present
	lpsz = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && lpsz[nLen-1] == '\n')
		rString.GetBufferSetLength(nLen-1);

	return nLen != 0;
}

UINT QFile::Write(const void* lpBuf, UINT nCount)
{
	if ( m_hFile == INVALID_HANDLE_VALUE)
		return 0;

    DWORD byteswritten;
    if ( !WriteFile( m_hFile, lpBuf, nCount, &byteswritten, NULL))
		return 0;
	else
		return byteswritten;
}

BOOL QFile::WriteString(LPCSTR lpsz)
{
	if ( lpsz == NULL || m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL ret = TRUE;
	for ( UINT n = 0; lpsz[n] != NULL; n++) {
		if ( lpsz[n] == '\n') {
			if ( Write( "\r\n", 2) != 2) {
				ret = FALSE;

				break;
			}
		} else {
			if ( Write( &lpsz[n], 1) != 1) {
				ret = FALSE;

				break;
			}
		}
	}

	return ret;
}

BOOL QFile::Seek(LONGLONG lOff)
{
	if ( m_hFile == INVALID_HANDLE_VALUE || !IsSeekable())
		return FALSE;

    LONG high = (LONG)(lOff >> 32);
    DWORD low = SetFilePointer( m_hFile, (LONG)(lOff & (LONGLONG)0xFFFFFFFF), &high, FILE_BEGIN);
    if ( ( low == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR ))
		return FALSE;
	else
		return ( (LONGLONG)high << 32 | low ) == lOff;
}

LONGLONG QFile::GetCurPtr(void)
{
	if ( m_hFile == INVALID_HANDLE_VALUE || !IsSeekable())
		return -1;

    LONG high = 0;
    DWORD low = SetFilePointer( m_hFile, 0, &high, FILE_CURRENT );
    if ( low == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		return -1;
	else
		return ( (LONGLONG)high << 32 ) | low;
}

LONGLONG QFile::GetLength(void)
{
	if ( m_hFile == INVALID_HANDLE_VALUE || !IsSeekable())
		return -1;

    DWORD high = 0;
    DWORD low = GetFileSize( m_hFile, &high);
    if ( (low == INVALID_FILE_SIZE) && (GetLastError() != NO_ERROR) ) return -1;
    return ( ( (LONGLONG)high << 32 ) | low );
}

BOOL QFile::IsSeekable(void)
{
    return m_hFile != INVALID_HANDLE_VALUE && GetFileType(m_hFile) == FILE_TYPE_DISK;
}

BOOL PASCAL QFile::FileExists(LPCTSTR lpszFileName)
{
	WIN32_FIND_DATA fd;
	HANDLE handle;

	handle = FindFirstFile( lpszFileName, &fd);
	if ( handle != INVALID_HANDLE_VALUE) {
		FindClose( handle);

		return TRUE;
	} else {
		return FALSE;
	}
}

