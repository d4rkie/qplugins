//-----------------------------------------------------------------------------
//
// File:	AudioInfo.cpp
//
// Authors:	Toke Noer (toke@noer.it)
//	Copyright (C) 2007
//
// About: Holds infomation of a playing track
//        Information should be UTF8 formattet
//-----------------------------------------------------------------------------

#include "Precompiled.h"
#include "AudioInfo.h"


//-----------------------------------------------------------------------------
// Constructor/destructor
CAudioInfo::CAudioInfo(void)
{
	m_strEmpty[0] = NULL; m_strEmpty[1] = NULL;
	m_szSource[0] = _T('U'); m_szSource[1] = NULL;

	m_strArtist = m_strEmpty;
	m_strTitle = m_strEmpty;
	m_strAlbum = m_strEmpty;
	m_strMbTrkId = m_strEmpty;
}

CAudioInfo::CAudioInfo(CAudioInfo* ai)
{
	m_strEmpty[0] = NULL; m_strEmpty[1] = NULL;
	m_szSource[0] = _T('U'); m_szSource[1] = NULL;

	m_strArtist = m_strEmpty;
	m_strTitle = m_strEmpty;
	m_strAlbum = m_strEmpty;
	m_strMbTrkId = m_strEmpty;

	this->SetAlbum(ai->GetAlbum());
	this->SetArtist(ai->GetArtist());
	this->SetMusicBrainTrackId(ai->GetMusicBrainTrackId());
	this->SetRating(ai->GetRating());
	this->SetSource(ai->GetSource());
	this->SetStartTime(ai->GetStartTime());
	this->SetTitle(ai->GetTitle());
	this->SetTrackLength(ai->GetTrackLength());
	this->SetTrackNumber(ai->GetTrackNumber());
}

CAudioInfo::~CAudioInfo(void)
{
	// Clean up all the strings
	if (m_strArtist != m_strEmpty)
		delete [] m_strArtist;
	if (m_strTitle != m_strEmpty)
		delete [] m_strTitle;
	if (m_strAlbum != m_strEmpty)
		delete [] m_strAlbum;
	if (m_strMbTrkId != m_strEmpty)
		delete [] m_strMbTrkId;
}


//-----------------------------------------------------------------------------
// Get functions
//-----------------------------------------------------------------------------

LPCSTR CAudioInfo::GetArtist()
{
	return m_strArtist;
}

LPCSTR CAudioInfo::GetTitle()
{
	return m_strTitle;
}

LPCSTR CAudioInfo::GetStartTime()
{
	return m_szStartTime;
}

LPCSTR CAudioInfo::GetSource()
{
	return m_szSource;
}

LPCSTR CAudioInfo::GetRating()
{
	return m_szRating;
}

INT CAudioInfo::GetTrackLength()
{
	return m_nTrackLength;
}

LPCSTR CAudioInfo::GetAlbum()
{
	return m_strAlbum;
}

INT CAudioInfo::GetTrackNumber()
{
	return m_nTrackNumber;
}

LPCSTR CAudioInfo::GetMusicBrainTrackId()
{
	return m_strMbTrkId;
}

//-----------------------------------------------------------------------------
// Set functions
//-----------------------------------------------------------------------------

void CAudioInfo::SetString(char** strDest, size_t nLen, LPCSTR strSrc)
{
	if (*strDest && *strDest != m_strEmpty) {
		delete [] *strDest;
		*strDest = NULL;
	}

	if (strSrc == NULL) {
		*strDest = m_strEmpty;
		return;
	}

	if (nLen == -1)
		nLen = strlen(strSrc);

	*strDest = new char[nLen + 1];
	strcpy_s(*strDest, nLen + 1, strSrc);
}

//-----------------------------------------------------------------------------

void CAudioInfo::SetArtist(LPCSTR str)
{
	SetString(&m_strArtist, -1, str);
}

void CAudioInfo::SetTitle(LPCSTR str)
{
	SetString(&m_strTitle, -1, str);
}

void CAudioInfo::SetStartTime()
{
	time_t ltime; // Needs 20 bit in order 10
	time(&ltime);

	// Below will throw access violation if buffer too small
	_ui64toa_s(ltime, m_szStartTime, NUMOFTCHARS(m_szStartTime), 10);
}

bool CAudioInfo::IsLegalNumber(char *str)
{
	for (size_t i = 0; i < strlen(str); i++) {
		if ((str[i] < '0') || (str[i] > '9'))
			return false;
	}
	return true; 
}

void CAudioInfo::SetStartTime(LPCSTR str)
{
	if (!IsLegalNumber(const_cast<char*>(str)))
		strcpy_s(m_szStartTime, NUMOFTCHARS(m_szStartTime), "");
	else
		strcpy_s(m_szStartTime, NUMOFTCHARS(m_szStartTime), str);
}

void CAudioInfo::SetSource(LPCSTR str)
{
	// P - Chosen by the user
	// R - Non-personalised broadcast (e.g. Shoutcast, BBC Radio 1)
	// E - Personalised recommendation except Last.fm (e.g. Pandora, Launchcast)
	// L - Last.fm (any mode). In this case, the 5-digit Last.fm recommendation key must be appended to this source ID to prove the validity of the submission.
	// U - Source unknown
	
	if (str)
		m_szSource[0] = *str;
	else
		m_szSource[0] = 'U';
}

void CAudioInfo::SetRating(LPCSTR str)
{
	// L - Love
	// B - Ban
	// S - Skip (only if source=L)
	if (str)
		m_szRating[0] = *str;
	else
		m_szRating[0] = NULL;
	m_szRating[1] = NULL;
}

void CAudioInfo::SetTrackLength(INT nLength)
{
	m_nTrackLength = nLength;
}

void CAudioInfo::SetTrackLength(LPCSTR str)
{
	if (str)
		m_nTrackLength = atoi(str);
	else
		m_nTrackLength = 0;
}

void CAudioInfo::SetAlbum(LPCSTR str)
{
	SetString(&m_strAlbum, -1, str);
}

void CAudioInfo::SetTrackNumber(INT nTrack)
{
	m_nTrackNumber = nTrack;	
}

void CAudioInfo::SetTrackNumber(LPCSTR str)
{
	if (str)
		m_nTrackNumber = atoi(str);
	else
		m_nTrackNumber = 0;
}

void CAudioInfo::SetMusicBrainTrackId(LPCSTR str)
{
	SetString(&m_strMbTrkId, -1, str);
}

//-----------------------------------------------------------------------------