#ifndef QMPSongInfo_H
#define QMPSongInfo_H

#include <Windows.h>
#include <IQCDMediaInfo.h>


class CSongInfo
{
public:
	CSongInfo();
	CSongInfo(WCHAR* strFile);
	~CSongInfo();

	void Destroy();

	BOOL IsValid();

	LPCWSTR GetArtist();
	LPCWSTR GetAlbum();
	LPCWSTR GetBitrate();
	LPCWSTR GetComment();
	LPCWSTR GetComposer();
	LPCWSTR GetFileFormat();
	LPCWSTR GetFilePath();
	LPCWSTR GetGenre();
	LPCWSTR GetLength();
	LPCWSTR GetLengthMs();
	LPCWSTR GetRating();
	LPCWSTR GetTitle();
	LPCWSTR GetTrackNumber();
	LPCWSTR GetYear();	

private:
	static const DWORD PAGE_SIZE;

	IQCDMediaInfo* m_pInfo;
	BOOL m_bIsValid;
	
	WCHAR* m_strBuffer;
	DWORD m_nBufferSize;

	void Init(WCHAR* strFile);
	void GetInfoByName(WCHAR* strQuery);
};

#endif // QMPSongInfo_H