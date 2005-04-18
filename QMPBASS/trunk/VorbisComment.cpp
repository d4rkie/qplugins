// VorbisComment.cpp: implementation of the VorbisComment class.
//
// Author: Toke Noer (toke@noer.it) April 2005
// 
// Usefull info:
//  http://www.xiph.org/ogg/vorbis/doc/Vorbis_I_spec.html#id4782005
//  http://www.xiph.org/ogg/vorbis/doc/v-comment.html
//  http://www.ogghelp.com/ogg/board/viewtopic.php?p=365&sid=94fc29c6859bc5ac94da3dee5ce8c90a
//////////////////////////////////////////////////////////////////////

#include "VorbisComment.h"
#include "tags.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VorbisComment::VorbisComment(reader* pFile, file_info* pInfo) :
	m_pFile(pFile),
	m_pInfo(pInfo)
{

}

VorbisComment::~VorbisComment()
{

}

//////////////////////////////////////////////////////////////////////

int VorbisComment::ReadTags()
{
	DWORD	nBufferSize = 1024;
	DWORD	nLen		= 0;
	DWORD	nListLength = 0;
	char	strLen[4];
	char*	pBuff		= NULL;
	char*	pEqualPos	= NULL;

	int nOffSet = GetCommentsOffset();
	if (!nOffSet)
		return 0;

	m_pFile->seek(nOffSet);

	// Create default size buffer, instead of dynamic buffers
	nBufferSize = IncBufferSize(&pBuff, nBufferSize);

	// Read vendorstring
	m_pFile->read(strLen, 4);
	nLen = (strLen[3] << 24) + (strLen[2] << 16) + (strLen[1] << 8) + strLen[0];
	if (nLen > nBufferSize)
		nBufferSize = IncBufferSize(&pBuff, nLen + 1);
	m_pFile->read(pBuff, nLen);
	pBuff[nLen] = 0;
	
	// user_comment_list_length
	m_pFile->read(strLen, 4);
	nListLength = (strLen[3] << 24) + (strLen[2] << 16) + (strLen[1] << 8) + strLen[0];
	if (nListLength == 0)
		return 0;

	// Iterate through all the field->value pairs
	for (DWORD i = 0; i < nListLength; i++)
	{
		m_pFile->read(strLen, 4);
		nLen = (strLen[3] << 24) + (strLen[2] << 16) + (strLen[1] << 8) + strLen[0];
		if (nLen > nBufferSize)
			nBufferSize = IncBufferSize(&pBuff, nLen + 1);
		m_pFile->read(pBuff, nLen);
		pBuff[nLen] = 0;

		// Find =
		pEqualPos = strstr(pBuff, "=");
		if (pEqualPos) {
			*pEqualPos = 0;						// Devide field-value
			insert_tag_field(m_pInfo, pBuff, pEqualPos+1);
		}
	}

	delete [] pBuff;

	return 1;
}

//////////////////////////////////////////////////////////////////////
// Gets the comments header offset from the beginning of the file
//////////////////////////////////////////////////////////////////////
int VorbisComment::GetCommentsOffset()
{
	int i = 0;
	int iRet = 0;
	char strTmp[256];			// Temp buffer
	char* pTmp = strTmp;

	m_pFile->seek(58);			// Skip first page
	iRet = m_pFile->read(pTmp, sizeof(strTmp)); // Check page boundry
	if (iRet < sizeof(strTmp) || strncmp(pTmp, "OggS", 4) != 0)
		return 0;				// Failed - Probably not a ogg file
	pTmp+=25;					// Ogg header

	while (i++ < 220 && 
		*(++pTmp) != 'v' && *(pTmp+1) != 'o' && *(pTmp+2) != 'r' &&
		*(pTmp+3) != 'b' && *(pTmp+4) != 'i' && *(pTmp+5) != 's')

	if (i >= 220)				// vorbis not found
		return 0;
	pTmp += 6;					// Start of comment field :)

	return pTmp - strTmp + 58;	// Comments offset
}

//////////////////////////////////////////////////////////////////////
// Memory management function
//////////////////////////////////////////////////////////////////////
DWORD VorbisComment::IncBufferSize(char** pBuff, const DWORD nSize)
{
	if (nSize <= 0)
		return 0;
	if (*pBuff)
		delete [] *pBuff;

	*pBuff = new char[nSize];

	if (*pBuff)
		return nSize;
	else
		return 0;
}

/*
  1) [vendor_length] = read an unsigned integer of 32 bits
  2) [vendor_string] = read a UTF-8 vector as [vendor_length] octets
  3) [user_comment_list_length] = read an unsigned integer of 32 bits
  4) iterate [user_comment_list_length] times {

       5) [length] = read an unsigned integer of 32 bits
       6) this iteration's user comment = read a UTF-8 vector as [length] octets

     }

  7) [framing_bit] = read a single bit as boolean
  8) if ( [framing_bit]  unset or end of packet ) then ERROR
  9) done.
*/