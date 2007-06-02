//-----------------------------------------------------------------------------
// Purpose:
//   This string class is written as an easy way to handle multibyte, unicode
//   and uft8 encoded strings.
//   The class is build on a std::wstring, so the internal representation
//   is unicode. (And thus no convertion is made for unicode calls)
//
// Usage:
//   The class is base on std::wstring and the internal format is always unicode.
//   The new function names a following Microsoft naming convention and not STL.
//   This is done on purpose, to distingues between inherited and new functions.
//
//   Multibyte and UFT8 representation will be kept in memory for fast
//   access after the first access. Use FreeMultiByte() and FreeUFT8()
//   to free those memories after use, if you don't plan to use them again.
//
// Warning: The Set* functions should be used for setting strings!
//   (The overloads should handle this in the future)   
//
// Note:
//   This string class should not be confused with Qt's QString class!
//   They are in no way related and properly work in diffent ways.
//
// Copyright (C) 2006:
//   Toke Noer Nøttrup
//   toke "at" noer.it
//
//   The code is free and can be used for any purpose as long as this header is
//   kept.
//   If bugs, memory leaks etc. is found, feel free to pay back the code by
//   providing me with the fixes.
//
//   The code is provided "as is" and WITHOUT ANY WARRANTY.
//
//-----------------------------------------------------------------------------

//#include "Precompiled.h"

#include <malloc.h>
#include "QString.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

QString::QString()
	: m_strMultiByte(NULL), m_strUTF8(NULL)
{
}
//-----------------------------------------------------------------------------

// Unicode constructor
QString::QString(LPCWSTR str)
	: m_strMultiByte(NULL), m_strUTF8(NULL)
{
	SetUnicode(str);

}
//-----------------------------------------------------------------------------

// MultiByte constructor
QString::QString(LPCSTR str)
	: m_strMultiByte(NULL), m_strUTF8(NULL)
{
	SetMultiByte(str);
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
QString::~QString(void)
{
	FreeMultiByte();
	FreeUTF8();
}



//-----------------------------------------------------------------------------
// Getter functions
//-----------------------------------------------------------------------------
LPCWSTR QString::GetUnicode()
{
	return this->c_str();
}
//-----------------------------------------------------------------------------

LPCSTR QString::GetMultiByte()
{
	if (!m_strMultiByte)
		ConvertWideToMultiByte(CP_THREAD_ACP, &m_strMultiByte);
	return m_strMultiByte;
}

//-----------------------------------------------------------------------------

LPCSTR QString::GetUTF8()
{
	if (!m_strUTF8)
		ConvertWideToMultiByte(CP_UTF8, &m_strUTF8);
	return m_strUTF8;
}
//-----------------------------------------------------------------------------

LPCTSTR QString::GetTStr()
{
#if defined(UNICODE) || defined(_UNICODE)
	return GetUnicode();
#else
	return GetMultiByte();
#endif
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Setter functions
//-----------------------------------------------------------------------------
_inline
void QString::SetUnicode(LPCWSTR str)
{
	FreeMultiByte();
	FreeUTF8();
	this->assign(str);
}
//-----------------------------------------------------------------------------

void QString::SetMultiByte(LPCSTR str)
{
	FreeMultiByte();
	FreeUTF8();

	ConvertAndSetToWide(CP_THREAD_ACP, str);
}
//-----------------------------------------------------------------------------

void QString::SetUTF8(LPCSTR str)
{
	FreeMultiByte();
	FreeUTF8();

	ConvertAndSetToWide(CP_UTF8, str);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Append functions
// This is functions that work like assign
//-----------------------------------------------------------------------------
void QString::AppendMultiByte(LPCSTR str)
{
	QString qstr;
	qstr.SetMultiByte(str);
	this->append(qstr);
}

//-----------------------------------------------------------------------------
// Free functions
//-----------------------------------------------------------------------------
_inline
void QString::FreeMultiByte()
{
	if (m_strMultiByte)
		_DELETE_ARRAY(m_strMultiByte);
}
//-----------------------------------------------------------------------------

_inline
void QString::FreeUTF8()
{
	if (m_strUTF8)
		_DELETE_ARRAY(m_strUTF8);
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Private helper functions
//-----------------------------------------------------------------------------
void QString::ConvertWideToMultiByte(UINT CodePage, LPSTR* str)
{
	int nByteSize = WideCharToMultiByte(CodePage, 0, this->c_str(), this->length(), NULL, 0, NULL, NULL);

	*str = new CHAR[nByteSize + 1];
	WideCharToMultiByte(CodePage, 0, this->c_str(), this->length(), *str, nByteSize, NULL, NULL);
	(*str)[nByteSize] = 0;
}
//-----------------------------------------------------------------------------

void QString::ConvertAndSetToWide(UINT CodePage, LPCSTR str)
{
	BOOL bStackAlloc = FALSE;
	WCHAR* pNewStr = NULL;
	int nWideChars = 0;
	int nStrLen = strlen(str);	// Gets len in bytes

	// Get number of widechars needed
	nWideChars = MultiByteToWideChar(CodePage, 0, str, nStrLen, NULL, 0);

	// Allocate memory for temp storage
	if (nWideChars < 1024)
	{  // Allocate on the stack - fast
		bStackAlloc = TRUE;
		// SafeAllocA
		pNewStr = (WCHAR*)_malloca( (nWideChars + 1)*sizeof(WCHAR) );
	}
	else // Allocate on the heap - slower
		pNewStr = new WCHAR[nWideChars + 1];

	if (!pNewStr)
		return;

	// Get the string and copy to our std::wstring object
	if (MultiByteToWideChar(CodePage, 0, str, nStrLen, pNewStr, nWideChars))
	{
		pNewStr[nWideChars] = 0;
		this->assign(pNewStr);
	}

	// Delete our temp storage
	if (bStackAlloc) {
		_freea(pNewStr); pNewStr = NULL; }
	else
		_DELETE_ARRAY(pNewStr);
}