#ifndef __QSTRING_H
#define __QSTRING_H

#if _MSC_VER > 1000
#pragma once
#endif
//-----------------------------------------------------------------------------

#ifndef _FREE
	#define _FREE(x)         {free(x); x = NULL;}
	#define _DELETE(x)       {delete x; x = NULL;}
	#define _DELETE_ARRAY(x) {delete [] x; x = NULL;}
	#pragma message ("_FREE(x), _DELETE(x), _DELETE_ARRAY(x) defined")
#endif

#define NUMOFTCHARS(x) sizeof(x)/sizeof(TCHAR)

//-----------------------------------------------------------------------------

#include <windows.h>
#include <string>

class QString : public std::wstring
{
public:
	QString();
	QString(LPCSTR str);
	QString(LPCWSTR str);
	~QString(void);

	void AppendMultiByte(LPCSTR str);

	LPCSTR  GetMultiByte();
	LPCSTR  GetUTF8();
	LPCWSTR GetUnicode();
	LPCTSTR GetTStr();

	void SetUnicode(LPCWSTR str);
	void SetMultiByte(LPCSTR str);
	void SetUTF8(LPCSTR str);

	void FreeMultiByte();
	void FreeUTF8();

public:
	inline operator LPCSTR  (void) { return this->GetMultiByte(); }
	inline operator LPCWSTR (void) { return GetUnicode(); }
	inline bool IsEmpty() { return this->empty(); }
	
//protected:

private:
	void ConvertWideToMultiByte(UINT CodePage, LPSTR* str);
	void ConvertAndSetToWide(UINT CodePage, LPCSTR str);

	LPSTR m_strMultiByte;
	LPSTR m_strUTF8;
};

#endif // __QSTRING_H