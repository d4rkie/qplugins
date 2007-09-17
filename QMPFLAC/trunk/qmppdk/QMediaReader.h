#pragma once

#include <atlstr.h>

#include "IQCDMediaSource.h"


//////////////////////////////////////////////////////////////////////////

class QMediaReader
{
public:
	QMediaReader(IQCDMediaSource * pms = NULL)
	{
		Attach( pms);
	}
	virtual ~QMediaReader()
	{
		if ( m_pms) {
			m_pms->CloseMedia(0);
			m_pms->Release();
		}
		if ( m_hKillReadingLoop) CloseHandle( m_hKillReadingLoop);
	}

	// Interfaces
public:
	BOOL Attach(IQCDMediaSource * pms)
	{
		m_pms = pms;
		m_LastError = MSERROR_SUCCESS;
		m_hKillReadingLoop = CreateEvent( NULL, FALSE, FALSE, NULL);

		return (m_pms && m_hKillReadingLoop);
	}
	BOOL Open(LONG flags = 0)
	{
		if ( m_pms) {
			m_LastError = m_pms->OpenMedia( flags);
			return (MSERROR_SUCCESS == m_LastError);
		} else {
			m_LastError = MSERROR_FAILED;
			return FALSE;
		}
	}
	BOOL Close(LONG flags = 0)
	{
		if ( m_pms) {
			m_LastError = m_pms->CloseMedia( flags);
			return (MSERROR_SUCCESS == m_LastError);
		} else {
			m_LastError = MSERROR_FAILED;
			return FALSE;
		}
	}
	virtual DWORD Read(LPBYTE buffer, LONG size, LONG flags = 0)
	{
		if ( m_pms) {
			DWORD bufferPos = 0;
			long readSize = size;
			BOOL finished = FALSE;

			m_LastError = MSERROR_SUCCESS;
			do {
				long bytesRead = 0;
				m_LastError = m_pms->ReadBytes( &buffer[bufferPos], readSize, &bytesRead, flags);
				switch ( m_LastError)
				{
				case MSERROR_SUCCESS:
					{
						bufferPos += bytesRead; readSize -= bytesRead;
						finished = (bufferPos == size);
					} break;

				case MSERROR_SOURCE_BUFFERING:
					{
						finished = (WAIT_OBJECT_0 == WaitForSingleObject( m_hKillReadingLoop, 0));
						if ( !finished) Sleep(10);
					} break;

				case MSERROR_SOURCE_EXHAUSTED:
					{
						finished = TRUE;
					} break;

				default:
					{
						finished = TRUE; // Oops!!
					} break;
				}
			} while ( !finished);
						return bufferPos;
		} else {
			m_LastError = MSERROR_FAILED;
			return 0;
		}
	}
	virtual BOOL Seek(ULONGLONG seekPos = 0)
	{
		if ( m_pms) {
			ResetEvent( m_hKillReadingLoop); // release the killing signal before seeking
			m_LastError = m_pms->SeekToByte( seekPos);
			return (MSERROR_SUCCESS == m_LastError);
		} else {
			m_LastError = MSERROR_FAILED;
			return FALSE;
		}
	}
	virtual LONGLONG GetPosition(LONG flags = 0)
	{
		if ( m_pms) {
			m_LastError = MSERROR_SUCCESS;
			__int64 pos = m_pms->GetSourcePosition( flags);
			return (pos >= 0) ? pos : 0;
		} else {
			m_LastError = MSERROR_FAILED;
			return 0;
		}
	}
	virtual ULONGLONG GetSize(LONG flags = 0)
	{
		if ( m_pms) {
			m_LastError = MSERROR_SUCCESS;
			__int64 len = m_pms->GetSourceSize( flags);
			return (len >= 0) ? len : 0;
		} else {
			m_LastError = MSERROR_FAILED;
			return 0;
		}
	}
	virtual BOOL CanSeek()
	{
		if ( m_pms) {
			m_LastError = MSERROR_SUCCESS;
			return (m_pms->GetMediaProperties() & MEDIASOURCE_PROP_CANSEEK);
		} else {
			m_LastError = MSERROR_FAILED;
			return FALSE;
		}
	}
	virtual BOOL IsEOC()
	{
		if ( m_pms) {
			m_LastError = MSERROR_SUCCESS;
			long props = m_pms->GetMediaProperties();
			if ( props & MEDIASOURCE_PROP_CANSEEK)
				return !(m_pms->GetSourcePosition(0) < m_pms->GetSourceSize(0));
			else if ( props & MEDIASOURCE_PROP_INTERNETSTREAM)
				return FALSE; // for non-seekable internet stream -- endless
			else
				return TRUE; // Oops!!
		} else {
			m_LastError = MSERROR_FAILED;
			return FALSE;
		}
	}
	virtual CString GetName()
	{
		if ( m_pms) {
			m_LastError = MSERROR_SUCCESS;
			return m_pms->GetMediaName();
		} else {
			m_LastError = MSERROR_FAILED;
			return _T("");
		}
	}
	long GetProperties()
	{
		if ( m_pms) {
			m_LastError = MSERROR_SUCCESS;
			return m_pms->GetMediaProperties();
		} else {
			m_LastError = MSERROR_FAILED;
			return 0;
		}
	}

public:
	long SetStatusCallback(IQCDMediaSourceStatus* pStatusCallback, long userData)
	{
		if ( m_pms) {
			m_LastError = MSERROR_SUCCESS;
			return m_pms->SetStatusCallback( pStatusCallback, userData);
		} else {
			m_LastError = MSERROR_FAILED;
			return 0;
		}
	}
	long GetAvailableMetadata(IQCDMediaInfo* pMediaInfo)
	{
		if ( m_pms) {
			m_LastError = MSERROR_SUCCESS;
			return m_pms->GetAvailableMetadata( pMediaInfo);
		} else {
			m_LastError = MSERROR_FAILED;
			return 0;
		}
	}

public:
	LONG GetLastError() const { return m_LastError; }
	void Reset() { SetEvent( m_hKillReadingLoop); }

public:
	IQCDMediaSource * m_pms; // the unique mediasource interface

protected:
	LONG m_LastError;

private:
	HANDLE m_hKillReadingLoop; // signal to kill reading loop when keeping in buffering
};

//////////////////////////////////////////////////////////////////////////

// a simple file reader
class QFileReader : public QMediaReader
{
public:
	QFileReader()
		: m_fp(NULL)
	{}
	~QFileReader()
	{ if ( m_fp) fclose( m_fp); }

public:
	BOOL Open(const CStringW fn)
	{
		m_fp = _wfopen( fn, L"rb");
		return (NULL != m_fp);
	}
	BOOL Close()
	{
		if ( m_fp) {
			fclose( m_fp);
			m_fp = NULL;
		}
	}

public:
	virtual DWORD Read(LPBYTE buffer, LONG size, LONG flags = 0)
	{
		if ( m_fp)
			return fread( buffer, 1, size, m_fp);
		else
			return 0;
	}
	virtual BOOL Seek(ULONGLONG seekPos)
	{
		if ( m_fp)
			return (0 == _fseeki64( m_fp, seekPos, SEEK_SET));
		else
			return FALSE;
	}
	virtual LONGLONG GetPosition(LONG flags = 0)
	{
		if ( m_fp)
			return _ftelli64( m_fp);
		else
			return -1L;
	}
	virtual ULONGLONG GetSize(LONG flags = 0)
	{
		if ( m_fp) {
			// save current position
			ULONGLONG prePos = _ftelli64( m_fp);
			// seek to the end of file
			if ( 0 != _fseeki64( m_fp, 0, SEEK_END))
				return 0;
			// get file size
			ULONGLONG retPos = _ftelli64( m_fp);
			// restore original position
			if ( 0 != _fseeki64( m_fp, prePos, SEEK_SET))
				return 0;
			// return file size
			return retPos;
		} else {
			return 0;
		}
	}
	virtual BOOL CanSeek()
	{
		return (NULL != m_fp);
	}
	virtual BOOL IsEOC()
	{
		if ( m_fp)
			return feof( m_fp);
		else
			return FALSE;
	}

private:
	FILE * m_fp;
};