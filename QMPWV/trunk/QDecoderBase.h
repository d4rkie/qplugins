///////////////////////////////////////////////////////////////////
// QDecoderBase Abstract Base Class for decoder input plug-in.
//
// Copyright 2006 QPlugins
// http://sourceforge.net/projects/qplugins
//
// Author: Shao Hao
////////////////////////////////////////////////////////////////////
#pragma once


#include "stdafx.h"

#include "QCDInputDLL.h"


class QDecoderBase
{
public:
	QDecoderBase(LPCTSTR lpszName, LPCTSTR lpszVersion, LPCTSTR lpszExtensions)
	: m_strExtensions(lpszExtensions)
	, m_bSeek(FALSE)
	, _nch(0), _srate(0), _bps(0), _br(0)
	, _pos(0)
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
		char foo[512];
		va_list args;
		va_start(args, lpszMsg);
		vsprintf(foo, lpszMsg, args);
		va_end(args);
		MessageBox( hwndPlayer, foo, "QDecoder Plug-in Error", MB_ICONSTOP);
	}

protected:
	CString m_strFullName, m_strExtensions;
	BOOL m_bSeek;

protected:
	int _nch; // number of channels
	int _srate; // samplerate
	int _bps; // bytes per second
	int _br; // bitrate

protected:
	unsigned int _pos; // current decoding position, in milliseconds
};

