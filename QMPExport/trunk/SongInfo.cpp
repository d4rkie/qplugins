#include <debug.h>

#include "QMPGeneralDLL.h"
#include "SongInfo.h"

//----------------------------------------------------------------------------
const DWORD CSongInfo::PAGE_SIZE = 4096;

CSongInfo::CSongInfo()
{
	// Default constructor - Get current playing song
	WCHAR strFile[MAX_PATH] = {0};
	QMPCallbacks.Service(opGetPlaylistFile, strFile, MAX_PATH*sizeof(WCHAR), -1);
	Init(strFile);
}

CSongInfo::CSongInfo(WCHAR* strFile)
{
	Init(strFile);
}

CSongInfo::~CSongInfo()
{
	Destroy();
}

void CSongInfo::Init(WCHAR* strFile)
{
	m_pInfo = NULL;
	m_bIsValid = FALSE;
	m_strBuffer = NULL;
	m_nBufferSize = 0;

	m_pInfo = (IQCDMediaInfo*)QMPCallbacks.Service(opGetIQCDMediaInfo, (void*)strFile, 0, 0);
	if (m_pInfo != NULL && m_pInfo->LoadFullData())
		m_bIsValid = TRUE;
}

void CSongInfo::Destroy()
{
	m_bIsValid = FALSE;

	// Release Interface
	if (m_pInfo) {
		m_pInfo->Release();
		m_pInfo = NULL;
	}
	delete [] m_strBuffer;
	m_strBuffer = NULL;
	m_nBufferSize = 0;
}

//----------------------------------------------------------------------------

BOOL CSongInfo::IsValid()
{
	return m_bIsValid;
}

//----------------------------------------------------------------------------

void CSongInfo::GetInfoByName(WCHAR* strQuery)
{
	// Query for size
	long nDataSize = 0;
	m_pInfo->GetInfoByName(QCDInfo_ArtistTrack, NULL, &nDataSize);
	
	if (nDataSize <= 0)
		return;

	// Allocate buffer space
	if (m_strBuffer == NULL || (DWORD)nDataSize > m_nBufferSize)
	{
		delete [] m_strBuffer; // Legal to delete null pointer

		m_nBufferSize = ((nDataSize / PAGE_SIZE) + 1) * PAGE_SIZE;

		m_strBuffer = new WCHAR[m_nBufferSize];
	}

	ASSERT(m_strBuffer != NULL);
	
	nDataSize = m_nBufferSize;
	if (!m_pInfo->GetInfoByName(strQuery, m_strBuffer, &nDataSize))
		m_strBuffer[0] = NULL;
}

//----------------------------------------------------------------------------

LPCWSTR CSongInfo::GetArtist()
{
	GetInfoByName(QCDInfo_ArtistTrack);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetAlbum()
{
	GetInfoByName(QCDInfo_TitleAlbum);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetBitrate()
{
	GetInfoByName(QCDInfo_Bitrate);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetComment()
{
	GetInfoByName(QCDInfo_Comment);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetComposer()
{
	GetInfoByName(QCDInfo_Composer);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetFilePath()
{
	return m_pInfo->GetMediaName();
}

LPCWSTR CSongInfo::GetFileFormat()
{
	GetInfoByName(QCDInfo_Stream_ContentType);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetGenre()
{
	GetInfoByName(QCDInfo_GenreTrack);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetLength()
{
	GetInfoByName(QCDInfo_DurationMS);
	// Remove last 3 chars to convert to seconds
	int i = 0;
	while (m_strBuffer[i++]) { }
	if (--i <= 3)
		return L"";
	m_strBuffer[--i] = NULL;
	m_strBuffer[--i] = NULL;
	m_strBuffer[--i]   = NULL;

	return m_strBuffer;
}

LPCWSTR CSongInfo::GetLengthMs()
{
	GetInfoByName(QCDInfo_DurationMS);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetRating()
{
	GetInfoByName(QCDInfo_Rating);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetTitle()
{
	GetInfoByName(QCDInfo_TitleTrack);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetTrackNumber()
{
	GetInfoByName(QCDInfo_TrackNumber);
	return m_strBuffer;
}

LPCWSTR CSongInfo::GetYear()
{
	GetInfoByName(QCDInfo_YearTrack);
	return m_strBuffer;
}