// VorbisComment.h: interface for the VorbisComment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VORBISCOMMENT_H__ACE49955_F552_40C5_BD42_D95275533CB6__INCLUDED_)
#define AFX_VORBISCOMMENT_H__ACE49955_F552_40C5_BD42_D95275533CB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "qcdhelper.h"

class VorbisComment  
{
public:
	VorbisComment(reader* pFile, file_info* pInfo);
	virtual ~VorbisComment();

	int ReadTags();
	int GetCommentsOffset();
private:
	DWORD IncBufferSize(char** pBuff, const DWORD nSize);
	reader* m_pFile;
	file_info* m_pInfo;
};

#endif // !defined(AFX_VORBISCOMMENT_H__ACE49955_F552_40C5_BD42_D95275533CB6__INCLUDED_)
