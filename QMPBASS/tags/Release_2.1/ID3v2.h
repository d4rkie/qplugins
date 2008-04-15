// VorbisComment.h: interface for the VorbisComment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ID3v2_H__ACE49955_F552_40C5_BD42_D95275533CB6__INCLUDED_)
#define AFX_ID3v2_H__ACE49955_F552_40C5_BD42_D95275533CB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "qcdhelper.h"

//////////////////////////////////////////////////////////////////////

struct ID3Header
{
	BOOL	bValid;
	WORD	nVersion;	// HIBYTE = major version, LOBYTE = revision
	BYTE	nFlags;
	DWORD	nSize;		// (total tag size - headersize(10))
};

struct ID3ExHeader
{
	BOOL	bValid;
	DWORD	nSize;		// Extended header size   $xx xx xx xx
	WORD	nFlags;		// Extended Flags         $xx xx
	DWORD	nPadding;	// Size of padding        $xx xx xx xx
	DWORD	nCRC;		// CRC32
};

struct ID3FrameHeader
{
	char	strFrameID[5]; // Frame ID   $xx xx xx xx  (four characters)
	DWORD	nSize;		// Size       $xx xx xx xx
	WORD	nFlags;		// Flags      $xx xx
};

//////////////////////////////////////////////////////////////////////

class ID3Frame
{
public:
	ID3Frame();
	virtual ~ID3Frame();

	void* GetDataPtr();
	DWORD GetSize();
	BOOL  SetSize(DWORD nSize);

private:
	void* m_pData;
	DWORD m_nSize;
};

//////////////////////////////////////////////////////////////////////

class ID3v2  
{
public:
	ID3v2(reader* pFile, file_info* pInfo);
	virtual ~ID3v2();

	BOOL GetHeader(ID3Header *pHeader, ID3ExHeader *pExHeader /* = NULL */);
	BOOL GetExtendedHeader(ID3ExHeader *pExHeader);
	DWORD GetNextFrameHeader(ID3FrameHeader *pFrame);
	BOOL GetNextFrame(ID3Frame *pFrame, ID3FrameHeader *pFrameHeader);

	void GetFrameTXXX(BYTE* pData, DWORD nFrameSize, BYTE** pRtnField, BYTE** pRtnValue, BYTE* nEncoding);

	void Convert16BitEndian(void* pData, const int iSize);

	reader* m_pFile;
	file_info* m_pInfo;
};

#endif // !defined(AFX_ID3v2_H__ACE49955_F552_40C5_BD42_D95275533CB6__INCLUDED_)