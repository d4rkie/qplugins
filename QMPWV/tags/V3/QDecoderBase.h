///////////////////////////////////////////////////////////////////
// QDecoderBase Abstract Base Class for decoder input plug-in.
//
// Copyright 2006 QPlugins
// http://sourceforge.net/projects/qplugins
//
// Author: Shao Hao
////////////////////////////////////////////////////////////////////
#pragma once

#include <atlstr.h>
#include "QCDModInput.h"
#include "IQCDMediaDecoder.h"

//////////////////////////////////////////////////////////////////////////

class QDecoderBase : public IQCDMediaDecoder
{
public:
	QDecoderBase(LPCTSTR lpszName, LPCTSTR lpszVersion, LPCTSTR lpszExtensions)
		: m_strExtensions(lpszExtensions)
		, m_bSeek(FALSE)
		, _nch(0), _srate(0), _bps(0), _br(0)
		, _pos(0)
		, _fn(_T(""))
	{
		m_strFullName = (lpszName && lstrlen( lpszName) > 0) ? lpszName : _T("QPlugins Decoder Module");
		m_strFullName += _T(" v");
		m_strFullName += (lpszVersion && lstrlen( lpszVersion) > 0) ? lpszVersion : _T(" v1.0");
	}
	virtual ~QDecoderBase(void)
	{
	}

public:
	virtual unsigned int GetDuration(LPCTSTR lpszFileName) = 0;
	virtual int IsSeekable(void) = 0;
	virtual int OpenFile(LPCTSTR lpszFileName) = 0;
	virtual int OpenStream(LPCTSTR lpszUrl, int startByte) = 0;
	virtual int Close(void) = 0;
	virtual int Decode(WriteDataStruct * wd) = 0;
	virtual int Seek(int ms) = 0;
	virtual int GetWaveFormFormat(WAVEFORMATEX * pwf) = 0;
	virtual int GetAudioInfo(AudioInfo * pai) = 0;

// for IQCDMediaDecoder interface
public:
	virtual void __stdcall Release(void)
	{
		delete this;
		return ;
	}

	virtual BOOL __stdcall StartDecoding(IQCDMediaDecoderCallback* pMDCallback, long userData)
	{
		WAVEFORMATEX wfex;
		WriteDataStruct wd;
		int ret;

		if ( _fn.IsEmpty()) // decoding filename is not available!
			return FALSE;

		if ( !OpenFile( _fn))
			return FALSE;

		// first, decode one block/frame to get wave format
		if ( Decode( &wd) > 0)
			GetWaveFormFormat( &wfex);
		else {
			pMDCallback->OnError( 1, userData);

			Close();

			return FALSE;
		}

		// running the decoding loop
		do {
			pMDCallback->OnReceive( (BYTE *)wd.data, wd.bytelen, &wfex, userData);
		} while ( (ret = Decode( &wd)) > 0);


		if ( ret < 0) // decoding error
			pMDCallback->OnError( 1, userData);
		else // finished decoding
			pMDCallback->OnEOF( userData);

		Close();

		return ret >= 0;
	}

public:
	LPCTSTR GetFullName(void) {	return m_strFullName; }
	LPCTSTR GetExtensions(void) { return m_strExtensions; }
	BOOL IsOurFile(LPCTSTR lpszFileName)
	{
		int pos= 0;
		BOOL found = FALSE;

		LPTSTR ext = PathFindExtension( lpszFileName);
		if ( !ext)
			return FALSE;
		else
			ext++; // skip . point to extension name

		CString token = m_strExtensions.Tokenize( _T(":"), pos);
		while ( !token.IsEmpty()) {
			if ( token.CompareNoCase( ext) == 0) {
				found = TRUE;

				break;
			} else {
				token= m_strExtensions.Tokenize( _T(":"), pos);
			}
		}

		return found;
	}
	unsigned int GetCurPos(void) { return _pos; }

protected:
	void _show_error(LPCTSTR lpszMsg, ...)
	{
		TCHAR foo[512];
		va_list args;
		va_start(args, lpszMsg);
		_vstprintf(foo, lpszMsg, args);
		va_end(args);
//		MessageBox( hwndPlayer, foo, "QDecoder Plug-in Error", MB_ICONSTOP);
	}

protected:
	CString m_strFullName, m_strExtensions; // Module description informations
	BOOL m_bSeek; // Is file/stream seekable?

protected:
	int _nch; // number of channels
	int _srate; // samplerate
	int _bps; // bytes per second
	int _br; // bitrate

protected:
	unsigned int _pos; // current decoding position, in milliseconds
	CString _fn; // current decoding filename for IQCDMediaDecoder. set by sub-class
};

