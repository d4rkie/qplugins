#include <TCHAR.h>
#include <Windows.h>
#include "QMPHelperGeneral.h"
#include "QCDGeneralDLL.h"

//-----------------------------------------------------------------------------
// QBlog functions
//
// When you play a song, the plugin records the following information into the registry.
// "Title" - The title of the song. If not defined, then it will be set to the current file name.
// "DurationString" - The running time of this track in the format %02d:%02d (ie minutes:seconds, both padded to two digits).
// "Author" - The author the track, if defined. Otherwise this does not exist.
// "Album" - The album of the track, if defined. Otherwise this does not exist.
// When you stop playing, that information is deleted. 
//-----------------------------------------------------------------------------
void QBlog_InsertInRegDB(PlayingSongInfo* pSongInfo)
{
	HKEY hKey = NULL;
	DWORD dwDis = NULL;
	LPTSTR lpClass = _T("");
	
	SECURITY_ATTRIBUTES lpSecurityAtt;
	lpSecurityAtt.nLength = sizeof(LPSECURITY_ATTRIBUTES);
	lpSecurityAtt.lpSecurityDescriptor = NULL;
	lpSecurityAtt.bInheritHandle = TRUE;

	if (IsPlayerStatus(QMP_PLAYING) && !IsEncoding())
	{
		if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\MediaPlayer\\CurrentMetadata"), 
			0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE,	&lpSecurityAtt, &hKey, &dwDis))
		{			

			if (pSongInfo->strArtist.Length() > 0)
				RegSetValueExW(hKey, L"Author", 0, REG_SZ, (LPBYTE)pSongInfo->strArtist.GetUnicode(), (pSongInfo->strArtist.Length()+1)* 2);

			if (pSongInfo->strAlbum.Length() > 0)
				RegSetValueExW(hKey, L"Album", 0, REG_SZ, (LPBYTE)pSongInfo->strAlbum.GetUnicode(), (pSongInfo->strAlbum.Length()+1)* 2);

			if (pSongInfo->strTitle.Length() > 0)
				RegSetValueExW(hKey, L"Title", 0, REG_SZ, (LPBYTE)pSongInfo->strTitle.GetUnicode(), (pSongInfo->strTitle.Length()+1)* 2);

			// Duration
			int iMinutes = pSongInfo->nLength / 60;
			int iSeconds = pSongInfo->nLength % 60;

			const int SIZEOFTIME = 128;
			TCHAR strTime[SIZEOFTIME] = {0};

			if (iMinutes < 10 && iSeconds < 10)
				_stprintf_s((LPTSTR)strTime, SIZEOFTIME, _T("0%d:0%d"), iMinutes, iSeconds);
			else if (iMinutes > 9 && iSeconds < 10)
				_stprintf_s((LPTSTR)strTime, SIZEOFTIME, _T("%d:0%d"), iMinutes, iSeconds);
			else if (iMinutes < 10 && iSeconds > 9)
				_stprintf_s((LPTSTR)strTime, SIZEOFTIME, _T("0%d:%d"), iMinutes, iSeconds);
			else
				_stprintf_s((LPTSTR)strTime, SIZEOFTIME, _T("%d:%d"), iMinutes, iSeconds);
		
			RegSetValueExW( hKey, _T("durationString"), 0, REG_SZ, (LPBYTE)strTime, (wcslen(strTime)+1) * sizeof(TCHAR) );

			// StartTime
			// LastRefresh 
			/*{
				SYSTEMTIME time;
				GetLocalTime(&time);
				swprintf_s(strUCS2, sizeof(strUCS2), L"%d:%d:%d", time.wHour, time.);
				RegSetValueExW(hKey, _T("StartTime"), 0, REG_SZ, (LPBYTE)strUCS2, cbSize);
			}*/

			// Close RegDB
			RegCloseKey(hKey);
		}
	}
}

//-----------------------------------------------------------------------------

void QBlog_CleanUpRegDB()
{
	HKEY hKey = NULL;

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\MediaPlayer\\CurrentMetadata"), 0,  KEY_WRITE, &hKey)) {
		RegDeleteValue(hKey, _T("Author"));
		RegDeleteValue(hKey, _T("Album"));
		RegDeleteValue(hKey, _T("Title"));
		RegDeleteValue(hKey, _T("durationString"));
	}			
}

//-----------------------------------------------------------------------------