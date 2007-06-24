#if !defined(AUDIOINFO_H)
#define AUDIOINFO_H

//#include <TCHAR.h>
//#include <Windows.h>

class CAudioInfo
{
public:
	CAudioInfo(void);
	//CAudioInfo(CAudioInfo const&);
	CAudioInfo(CAudioInfo* ai);

	~CAudioInfo(void);

	LPCSTR  GetAlbum();
	LPCSTR  GetArtist();
	LPCSTR  GetMusicBrainTrackId();
	LPCSTR  GetStartTime();
	LPCSTR  GetSource();
	LPCSTR  GetRating();
	LPCSTR  GetTitle();
	INT     GetTrackLength();
	INT     GetTrackNumber();

	void    GetCacheString(char** pStr, DWORD* nChars);

	void   SetArtist(LPCSTR str);
	void   SetArtist(LPCWSTR str);
	void   SetTitle(LPCSTR str);
	void   SetTitle(LPCWSTR str);
	void   SetAlbum(LPCSTR str);
	void   SetAlbum(LPCWSTR str);
	void   SetTrackNumber(INT nTrack);
	void   SetTrackNumber(LPCSTR str);
	void   SetTrackNumber(LPCWSTR str);
	void   SetMusicBrainTrackId(LPCSTR str);
	void   SetMusicBrainTrackId(LPCWSTR str);

	void   SetStartTime();
	void   SetStartTime(LPCSTR str);
	void   SetSource(LPCSTR str);
	void   SetRating(LPCSTR str);
	void   SetTrackLength(INT nLength);
	void   SetTrackLength(LPCSTR str);


private:
	void SetString(char** strDest, size_t nLen, LPCSTR strSrc);
	void SetStringW(char** strDest, size_t nLen, LPCWSTR strSrc);

	bool CAudioInfo::IsLegalNumber(char *str);

	char  m_szSource[2];
	char  m_szRating[2];
	char* m_strArtist;
	char* m_strTitle;
	char* m_strAlbum;     // Empty if unknown
	char* m_strMbTrkId;   // MusicBrainz Track ID - Empty if unknown
	INT   m_nTrackLength; // Seconds
	INT   m_nTrackNumber; // Empty if unknown
	
	char  m_strEmpty[2];  // 1 byte align - Change in all constructors as well !!

	char  m_szStartTime[32]; // Unix timestamp, UTC timezone
};


#endif // !defined(AUDIOINFO_H)