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

#include "QMediaReader.h"


//////////////////////////////////////////////////////////////////////////

class QDecoderBase : public IQCDMediaDecoder
{
public:
	QDecoderBase(LPCWSTR lpszName, LPCWSTR lpszVersion, LPCWSTR lpszExtensions)
		: m_strExtensions(lpszExtensions)
		, _seekable(false)
		, _nch(0), _srate(0), _bps(0), _br(0)
		, _pos(0)
		, _pmr(NULL), _pmrDec(NULL)
	{
		m_strFullName = (lpszName && lstrlenW( lpszName) > 0) ? lpszName : L"QPlugins Decoder Module";
		m_strFullName += _T(" v");
		m_strFullName += (lpszVersion && lstrlenW( lpszVersion) > 0) ? lpszVersion : L" v2.0";
	}
	virtual ~QDecoderBase(void)
	{
	}

public:
	// Get the track's extents struct. DONOT open/close the passed mediasource interface
	virtual int GetTrackExtents(QMediaReader & mediaReader, TrackExtents & te) = 0;
	// Check the seek ability of media.
	virtual int IsSeekable(void) { return _seekable; }
	// Open the decoder. DONOT open/close the passed mediasource interface
	virtual int Open(QMediaReader & mediaReader) = 0;
	//virtual int OpenStream(LPCTSTR lpszUrl, int startByte) = 0;
	// Close the opened decoder.
	virtual int Close(void) = 0;
	// DO decoding basing on the passed mediasource interface in OpenDecoder.
	virtual int Decode(WriteDataStruct & wd) = 0;
	// DO seeking basing on the passed mediasource interface in OpenDecoder.
	virtual int Seek(int ms) = 0;
	// Get the wave format.
	virtual int GetWaveFormFormat(WAVEFORMATEX & wfex) = 0;
	// Get the audio informations
	virtual int GetAudioInfo(AudioInfo & ai) = 0;
	// Is our decoder active?
	virtual int IsActive(void) = 0;

// for IQCDMediaDecoder interface
public:
	IQCDMediaDecoder * CreateDecoderInstance(QMediaReader * pmr, int flags)
	{
		if ( _pmrDec) {
			delete _pmrDec;
			_pmrDec = NULL;
		}

		_pmrDec = pmr;

		return this;
	}

	virtual void __stdcall Release(void)
	{
		if ( _pmrDec) {
			delete _pmrDec;
			_pmrDec = NULL;
		}

		delete this;
	}

	virtual BOOL __stdcall StartDecoding(IQCDMediaDecoderCallback* pMDCallback, long userData)
	{
		if ( !pMDCallback || !_pmrDec)
			return FALSE;

		WAVEFORMATEX wfex;
		WriteDataStruct wd;

		// open media reader
		if ( !_pmrDec->Open(0))
			return FALSE;

		// open decoder
		if ( Open( *_pmrDec) <= 0) {
			_pmrDec->Close();
			return FALSE;
		}

		// do decoding
		while ( IsActive()) {
			if ( Decode( wd) > 0) {
				GetWaveFormFormat( wfex);
				int cb_ret = pMDCallback->OnReceive( (BYTE *)wd.data, wd.bytelen, &wfex, userData);
				if ( cb_ret <= 0) break;
			} else {
				pMDCallback->OnError( -1, userData);
			}
			Sleep(10);
		}

		// finish decoding
		pMDCallback->OnEOF( userData);

		// close decoder
		Close();

		// close media reader
		_pmrDec->Close();

		return TRUE;
	}

public:
	LPCWSTR GetFullName(void) {	return m_strFullName; }
	LPCWSTR GetExtensions(void) { return m_strExtensions; }
	BOOL IsOurFile(LPCWSTR lpszFileName)
	{
		int pos= 0;
		BOOL found = FALSE;

		LPWSTR ext = PathFindExtensionW( lpszFileName);
		if ( !ext)
			return FALSE;
		else
			ext++; // skip . point to extension name

		CStringW token = m_strExtensions.Tokenize( L":", pos);
		while ( !token.IsEmpty()) {
			if ( token.CompareNoCase( ext) == 0) {
				found = TRUE;

				break;
			} else {
				token= m_strExtensions.Tokenize( L":", pos);
			}
		}

		return found;
	}
	unsigned int GetCurPos(void) { return _pos; }

protected:
	CStringW m_strFullName, m_strExtensions; // Module description informations

protected:
	int _nch; // number of channels
	int _srate; // samplerate
	int _bps; // bits per sample
	int _br; // bitrate

protected:
	bool _seekable; // Is file/stream seekable?
	unsigned int _pos; // current decoding position, in milliseconds
	QMediaReader * _pmr; // decoding instance
	QMediaReader * _pmrDec; // decoding instance
};

