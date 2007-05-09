#ifndef QMPModule_H
#define QMPModule_H

#pragma warning( disable : 4356 4311 )

#include <atlstr.h>


//////////////////////////////////////////////////////////////////////////

template< typename T, typename TCallbacks >
class QMPModule
{
public:
	static T * GetModule()
	{
		static T inst;

		return &inst;
	}
	TCallbacks * ExportInterface()
	{
		return &QCDCallbacks;
	}

protected:
	QMPModule()
	{
		QCDCallbacks.size = sizeof(TCallbacks);
	}

protected:
	static HWND			hwndPlayer;   // The unique player window handle
	static TCallbacks	QCDCallbacks; // Module interface export to player

protected:
	static void UTF8toUCS2(const CStringA & utf8str, CStringW & ucs2str)
	{
		long ucs2len = utf8str.GetLength() + 1;
		ucs2str.Empty();
		QCDCallbacks.Service( opUTF8toUCS2, (LPVOID)(LPCSTR)utf8str, (long)ucs2str.GetBuffer( ucs2len), ucs2len);
		ucs2str.ReleaseBuffer();
	}
	static void UCS2toUTF8(const CStringW & ucs2str, CStringA & utf8str)
	{
		long utf8len = ucs2str.GetLength() * 3 + 1;
		utf8str.Empty();
		QCDCallbacks.Service( opUCS2toUTF8, (LPVOID)(LPCWSTR)ucs2str, (long)utf8str.GetBuffer( utf8len), utf8len);
		utf8str.ReleaseBuffer();
	}
	static void MBtoUCS2(const CStringA & mbstr, CStringW & ucs2str)
	{
		long ucs2len = mbstr.GetLength() + 1;
		ucs2str.Empty();
		//QCDCallbacks.Service( opMBtoUCS2, (void*)mbstr, (long)ucs2str, ucs2len);
		MultiByteToWideChar( CP_ACP, 0, mbstr, -1, ucs2str.GetBuffer( ucs2len), ucs2len);
		ucs2str.ReleaseBuffer();
	}
	static void UCS2toMB(const CStringW & ucs2str, CStringA & mbstr)
	{
		long mblen = ucs2str.GetLength() * 3 + 1;
		mbstr.Empty();
		//QCDCallbacks.Service( opUCS2toMB, (void*)ucs2str, (long)mbstr, mblen);
		WideCharToMultiByte( CP_ACP, 0, ucs2str, -1, mbstr.GetBuffer( mblen), mblen, NULL, NULL);
		mbstr.ReleaseBuffer();
	}
	static void MBtoUTF8(const CStringA & mbstr, CStringA & utf8str)
	{
		CStringW ucs2str;
		MBtoUCS2( mbstr, ucs2str);
		utf8str.Empty();
		UCS2toUTF8( ucs2str, utf8str);
	}
	static void UTF8toMB(const CStringA & utf8str, CStringA & mbstr)
	{
		CStringW ucs2str;
		UTF8toUCS2( utf8str, ucs2str);
		mbstr.Empty();
		UCS2toMB( ucs2str, mbstr);
	}
};

#endif //QMPModule_H

