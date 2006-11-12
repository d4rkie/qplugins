#ifndef QMPModule_H
#define QMPModule_H

#pragma warning( disable : 4356 )

//-----------------------------------------------------------------------------
// Helpers Macros
//-----------------------------------------------------------------------------

#define UTF8toUCS2(utf8str,ucs2str,ucs2len) QCDCallbacks.Service( opUTF8toUCS2, (void*)utf8str, (long)ucs2str, ucs2len)
#define UCS2toUTF8(ucs2str,utf8str,utf8len) QCDCallbacks.Service( opUCS2toUTF8, (void*)ucs2str, (long)utf8str, utf8len)
#define MBtoUCS2(mbstr,ucs2str,ucs2len) MultiByteToWideChar( CP_ACP, 0, mbstr, -1, ucs2str, ucs2len)//QCDCallbacks.Service( opMBtoUCS2, (void*)mbstr, (long)ucs2str, ucs2len)
#define UCS2toMB(ucs2str,mbstr,mblen) WideCharToMultiByte( CP_ACP, 0, ucs2str, -1, mbstr, mblen, NULL, NULL)//QCDCallbacks.Service( opUCS2toMB, (void*)ucs2str, (long)mbstr, mblen)
#define MBtoUTF8(mbstr,utf8cstring) \
{ \
	long ucs2len = (lstrlenA( mbstr) + 1) * 2; \
	WCHAR * ucs2str = new WCHAR[ucs2len]; \
	MBtoUCS2(mbstr,ucs2str,ucs2len); \
	UCS2toUTF8(ucs2str,utf8cstring.GetBuffer(ucs2len),ucs2len); \
	delete [] ucs2str; \
	utf8cstring.ReleaseBuffer(); \
}
#define UTF8toMB(utf8str,mbcstring) \
{ \
	long utf8len = lstrlenA( utf8str) + 1; \
	long ucs2len = utf8len * 2; \
	WCHAR * ucs2str = new WCHAR[ucs2len]; \
	UTF8toUCS2(utf8str,ucs2str,ucs2len); \
	UCS2toMB(ucs2str,mbcstring.GetBuffer(utf8len),utf8len); \
	delete [] ucs2str; \
	mbcstring.ReleaseBuffer(); \
}

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
	static TCallbacks * ExportInterface()
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
};

#endif //QMPModule_H

