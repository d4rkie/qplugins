// ID3v2.cpp: implementation of the ID3v2 class.
//
// About: ID3v2 is a ID3v2 reader class
//
// Author: Toke Noer (toke@noer.it) April 2005
// 
// Usefull info:
//   http://www.id3.org/id3v2.3.0.txt	Standard which is implemented
//   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/unicode_42jv.asp
//////////////////////////////////////////////////////////////////////

#include "ID3v2.h"
#include <stdio.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ID3v2::ID3v2(reader* pFile, file_info* pInfo) :
	m_pFile(pFile), m_pInfo(pInfo)
{

}

ID3v2::~ID3v2()
{

}

//////////////////////////////////////////////////////////////////////
// Get the id3v2 header
// See section: 3.1 ID3v2 header
//////////////////////////////////////////////////////////////////////
BOOL ID3v2::GetHeader(ID3Header *pHeader, ID3ExHeader *pExHeader = NULL)
{
	BYTE nHeader[10];
	INT iRet = 0;

	pHeader->bValid = false;

	// The ID3v2 tag header should be the first information in the file
	m_pFile->seek(0);
	iRet = m_pFile->read(&nHeader, sizeof(nHeader));
	if (iRet <= 0) return 0;

	// Check if ID3
	if (strncmp((const char*)nHeader, "ID3", 3) != 0)
		return 0;

	// Get version
	pHeader->nVersion = MAKEWORD(nHeader[4], nHeader[3]);
	if (pHeader->nVersion == MAKEWORD(0xff, 0xff))
		return 0;

	// Get flags
	pHeader->nFlags = nHeader[5];

	// Get size
	if (nHeader[6] < 0x80 && nHeader[7] < 0x80 && nHeader[8] < 0x80 && nHeader[9] < 0x80)
		pHeader->nSize = MAKELONG(MAKEWORD(nHeader[9], nHeader[8]), MAKEWORD(nHeader[7], nHeader[6]));
	else
		return 0;

	pHeader->bValid = true;

	if (pExHeader && ((pHeader->nFlags & 0x40) == 0x40) )
		GetExtendedHeader(pExHeader);

	return 1;
}

//////////////////////////////////////////////////////////////////////
// Get the extended header
// See section: 3.2 ID3v2 extended header
//  Note:
//   The extended header is considered separate from the header
//   proper, and as such is subject to unsynchronisation.
//////////////////////////////////////////////////////////////////////
BOOL ID3v2::GetExtendedHeader(ID3ExHeader *pExHeader)
{
	BYTE nExHeader[4];
	INT iRet = 0;

	pExHeader->bValid = false;

	// Read size
	iRet = m_pFile->read(&nExHeader, 4);
	if (iRet <= 0) return 0;
	if (nExHeader[0] < 0x80 && nExHeader[1] < 0x80 && nExHeader[2] < 0x80 && nExHeader[3] < 0x80)
		pExHeader->nSize = MAKELONG(MAKEWORD(nExHeader[3], nExHeader[2]), MAKEWORD(nExHeader[1], nExHeader[0]));

	// Read flags
	iRet = m_pFile->read(&nExHeader, 2);
	if (iRet <= 0) return 0;
	pExHeader->nFlags = MAKEWORD(nExHeader[1], nExHeader[0]);
	
	// Read padding size
	iRet = m_pFile->read(&nExHeader, 4);
	if (iRet <= 0) return 0;
	pExHeader->nPadding = MAKELONG(MAKEWORD(nExHeader[3], nExHeader[2]), MAKEWORD(nExHeader[1], nExHeader[0]));

	// If CRC32 value present, then read it	
	if ((pExHeader->nFlags & 0x8000) == 0x8000)
	{
		iRet = m_pFile->read(&nExHeader, 4);
		if (iRet <= 0) return 0;
		pExHeader->nCRC = MAKELONG(MAKEWORD(nExHeader[3], nExHeader[2]), MAKEWORD(nExHeader[1], nExHeader[0]));
	}

	pExHeader->bValid = true;

	return 1;
}

//////////////////////////////////////////////////////////////////////
// Get the id3v2 header
// See section: 3.3 ID3v2 frame overview
//////////////////////////////////////////////////////////////////////
DWORD ID3v2::GetNextFrameHeader(ID3FrameHeader *pFrame)
{
	BYTE nFrameHeader[10];

	if (m_pFile->read(&nFrameHeader, sizeof(nFrameHeader)) <= 0) return 0x7fffffff;

	// Frame ID
	pFrame->strFrameID[0] = nFrameHeader[0];
	pFrame->strFrameID[1] = nFrameHeader[1];
	pFrame->strFrameID[2] = nFrameHeader[2];
	pFrame->strFrameID[3] = nFrameHeader[3];
	pFrame->strFrameID[4] = 0;

	// Size
	pFrame->nSize = MAKELONG(MAKEWORD(nFrameHeader[7], nFrameHeader[6]),
							 MAKEWORD(nFrameHeader[5], nFrameHeader[4]) );
	// Flags
	pFrame->nFlags = MAKEWORD(nFrameHeader[9], nFrameHeader[8]);

	return pFrame->nSize + 10;
}

//////////////////////////////////////////////////////////////////////
// Get the frame data
//////////////////////////////////////////////////////////////////////
BOOL ID3v2::GetNextFrame(ID3Frame *pFrame, ID3FrameHeader *pFrameHeader)
{
	if (pFrameHeader->nSize <= 0) return false;

	if (!pFrame->SetSize(pFrameHeader->nSize)) return false;

	if (m_pFile->read(pFrame->GetDataPtr(), pFrame->GetSize()) <= 0)
		return false;

	return 1;
}

//////////////////////////////////////////////////////////////////////
// Convert from big/low endian
//////////////////////////////////////////////////////////////////////
void ID3v2::Convert16BitEndian(void* pData, const int iSize)
{
	WCHAR* p = (WCHAR*)pData;

	if (!p || iSize < 0 || iSize % 2 == 1)
		return;

	for (int i = 0; i < iSize / 2; i++)
		*(p + i) = (((*(p + i) >> 8)) | (*(p + i) << 8));
}

//////////////////////////////////////////////////////////////////////
// Get TXXX frame data
// *Remember to delete [] pField and pValue from calling function
//////////////////////////////////////////////////////////////////////
void ID3v2::GetFrameTXXX(BYTE* pData, DWORD nFrameSize, BYTE** _pRtnField, BYTE** _pRtnValue, BYTE* nEncoding)
{
	// Frame layout (unicode): |E|FE|FF| ... string ..|L'\0'|FE|FF| ... string ...
	DWORD nSize = 0;
	BYTE* pField	= 0;
	BYTE* pDevide	= 0;
	BYTE* pValue	= 0;

	BYTE* pRtnField = *_pRtnField;
	BYTE* pRtnValue = *_pRtnValue;

	*nEncoding = pData[0];
	pData++;					// Move to start of data

	if (*nEncoding == 1) { // Strings are unicode
		// Find devider between field and value
		pDevide = (BYTE*)wcschr((const WCHAR*)pData, L'\0');
		if (!pDevide) return;

		// Get field
		pField = pData;
		if (pField[0] == 0xFE && pField[1] == 0xFF) { // big endian
			pField += 2;
			Convert16BitEndian(pField, (BYTE*)pDevide - pField);
		}
		else if (pField[1] == 0xFE && pField[0] == 0xFF) // little endian
			pField += 2;

		nSize = pDevide - pField;
		pRtnField = new BYTE[nSize + 2];	// + L'\0'
		lstrcpynW((WCHAR*)pRtnField, (WCHAR*)pField, nSize / 2 + 1);
		pRtnField[nSize - 1] = 0;
		pRtnField[nSize] = 0;


		// Get value
		pValue = pDevide + 2;			// skip L'\0'
		nSize = nFrameSize - (pValue - (pData - 1));	// Size + BOM

		if (pValue[0] == 0xFE && pValue[1] == 0xFF) { // big endian
			pValue += 2;
			nSize -= 2;
			Convert16BitEndian(pValue, nSize);
		}
		else if (pValue[1] == 0xFE && pValue[0] == 0xFF) { // little endian
			pValue += 2;
			nSize -= 2;
		}
		
		pRtnValue = new BYTE[nSize + 2];
		lstrcpynW((WCHAR*)pRtnValue, (WCHAR*)pValue, nSize / 2 + 1);
		pRtnValue[nSize - 1] = 0;
		pRtnValue[nSize] = 0;
	} else if (*nEncoding == 0) {		// ASCII text
		// Find divider between field and value
		pDevide = (BYTE*)strchr((char *)pData, '\0');
		if (!pDevide) return;

		nSize = pDevide - pData;
		pRtnField = new BYTE[nSize + 1];	// + '\0'
		memcpy(pRtnField, pData, nSize + 1);

		// Get value
		pValue = pDevide + 1;			// skip '\0'
		nSize = nFrameSize - (pValue - (pData - 1));	// Size + BOM

		pRtnValue = new BYTE[nSize + 1];
		memcpy(pRtnValue, pValue, nSize);
		pRtnValue[nSize] = '\0';
	}

	*_pRtnField = pRtnField;
	*_pRtnValue = pRtnValue;
}

//////////////////////////////////////////////////////////////////////
// ID3Frame Class
//
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ID3Frame::ID3Frame()  :
	m_pData(NULL)
{

}

ID3Frame::~ID3Frame()
{
	if (m_pData)
		delete m_pData;
}

//////////////////////////////////////////////////////////////////////

void* ID3Frame::GetDataPtr()
{
	return m_pData;
}

DWORD ID3Frame::GetSize()
{
	return m_nSize;
}

BOOL ID3Frame::SetSize(DWORD nSize)
{
	if (m_pData)
		delete m_pData;
	
	m_pData = new BYTE[nSize + 2];	// Make room for an extra WCHAR
	if (m_pData)
		m_nSize = nSize;
	else {
		m_nSize = 0;
		return false;
	}

	return true;
}


/*int ID3v2::ReadTags(CPtrArray)
{
	DWORD nRead	= 0;

	ID3Header		header;
	ID3ExHeader		exheader;
	ID3FrameHeader	frameheader;
	ZeroMemory(&header, sizeof(ID3Header));
	ZeroMemory(&exheader, sizeof(ID3ExHeader));
	ZeroMemory(&frameheader, sizeof(ID3FrameHeader));

	if (!m_pFile || !m_pInfo)
		return 0;
	if (!GetHeader(&header, &exheader))
		return 0;
	if (!header.bValid || HIBYTE(header.nVersion) != 3)
		return 0;

	while ((nRead += GetNextFrameHeader(&frameheader)) <= header.nSize - exheader.nPadding)
	{
		// printf("nRead: %d <= %d (%d:%d)\n", nRead, header.nSize - exheader.nPadding, header.nSize, exheader.nPadding);
		if (frameheader.nSize > 0) {
			ID3Frame frame;

			if (GetNextFrame(&frame, &frameheader)) {
				PrintFrame(&frameheader, &frame);
			}
		}
		else
			nRead = 0x7fffffff;			
	}

	return 1;
}*/
