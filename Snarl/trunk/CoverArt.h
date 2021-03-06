// CoverArt.h
//
//////////////////////////////////////////////////////////////////////

#ifndef COVER_ART_H
#define COVER_ART_H

#include <deque>
#include <qcdtagdata.h>
#include "QCDGeneralDLL.h"

//-----------------------------------------------------------------------------
// Declarations
//-----------------------------------------------------------------------------

void CoverArtInitialize();
void CoverArtShutdown();

void GetCoverArt(long nIndex, QString* strIcon);
BOOL GetCoverArtFromFile(long nIndex, QString* strIcon);
BOOL GetCoverArtFromML(long nIndex, QString* strIcon);

QString _GetInfo(PluginServiceOp op, long nIndex);
QString _GetFilename(long nIndex);
QString _GetFileFolder(long nIndex);

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

QString g_strTempPath;
std::deque<QString*> g_listTempFiles;

//-----------------------------------------------------------------------------
// Initialization and shutdown
//-----------------------------------------------------------------------------

void CoverArtInitialize()
{
	size_t nReturnSize = 0;
	WCHAR szTmp[MAX_PATH] = {0};

	if (_wgetenv_s(&nReturnSize, szTmp, sizeof(szTmp)/sizeof(WCHAR), L"TEMP") == 0)
	{ // Success
		g_strTempPath.SetUnicode(szTmp);
		g_strTempPath.AppendUnicode(L"\\");
	}
}

//-----------------------------------------------------------------------------

void CoverArtShutdown()
{
	// Clean up the files we created
	for (std::deque<QString*>::iterator aiIter = g_listTempFiles.begin(); aiIter != g_listTempFiles.end( ); aiIter++)
	{
		QString strTmp;
		strTmp.SetUnicode(g_strTempPath);
		strTmp.AppendUnicode(**aiIter);
		DeleteFile(strTmp.GetUnicode());
		delete *aiIter;
	}
}


//-----------------------------------------------------------------------------
// GetCoverArt
//-----------------------------------------------------------------------------

void GetCoverArt(long nIndex, QString* strIcon)
{
	if (!GetCoverArtFromFile(nIndex, strIcon))
		GetCoverArtFromML(nIndex, strIcon);
}

//-----------------------------------------------------------------------------

BOOL GetCoverArtFromML(long nIndex, QString* strIcon)
{
	WCHAR strFile[MAX_PATH] = {0};
	
	Service(opGetPlaylistFile, strFile, MAX_PATH*sizeof(WCHAR), nIndex);

	IQCDTagInfo* pTagInfo = (IQCDTagInfo*)Service(opGetIQCDTagInfo, strFile, 0, 0);
	if (pTagInfo)
	{
		try
		{
			if (!pTagInfo->ReadFromFile(TAG_ALL))
				throw L"Failed on: pTagInfo->ReadFromFile(TAG_ALL)";

			QTAGDATA_TYPE tagType;
			DWORD valueLen = 0;

			pTagInfo->GetTagDataByName(QCDTag_Artwork, &tagType, 0, &valueLen, 0);
			if (valueLen > 0)
			{
				BYTE* pValue = new BYTE[valueLen];
				if (!pValue)
					throw L"Out of memory!";

				pTagInfo->GetTagDataByName(QCDTag_Artwork, &tagType, pValue, &valueLen, 0);
				QTAGDATA_HEADER_ARTWORK* pArtwork = (QTAGDATA_HEADER_ARTWORK*)pValue;

				// Create filename
				QString* strFilename = new QString();
				strFilename->AppendUnicode(_GetInfo(opGetArtistName, nIndex));
				strFilename->AppendUnicode(L" - ");
				strFilename->AppendUnicode(_GetInfo(opGetAlbumName, nIndex));
				strFilename->AppendUnicode(L".");
				
				if (wcscmp(pArtwork->pszMimeType, L"image/jpeg") == 0)
					strFilename->AppendUnicode(L"jpg");

				QString strFullPath;
				strFullPath.SetUnicode(g_strTempPath);
				strFullPath.AppendUnicode(strFilename->GetUnicode());

				// Output to temp file
				HANDLE hFile = CreateFile(strFullPath.GetUnicode(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					DWORD nWritten = 0;
					if (WriteFile(hFile, pArtwork->pbData, pArtwork->dwDataLen, &nWritten, NULL)) {
						strIcon->SetUnicode(strFullPath);
						g_listTempFiles.push_back(strFilename);
					}
					else
						delete strFilename;
					CloseHandle(hFile);
				}
				else
				{
					DWORD nErr = GetLastError();
					
					if (nErr == ERROR_FILE_EXISTS)
						strIcon->SetUnicode(strFullPath);
					else {
						LPVOID lpMsgBuf;
						FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, nErr, 0, (LPTSTR)&lpMsgBuf, 0, NULL);
						OutputDebugInfo(L"GetCoverArtFromML : CreateFile error : %s", lpMsgBuf);
						LocalFree(lpMsgBuf);
					}
					delete strFilename; // Not added to list, so delete
				}

				delete [] pValue;
			}
		}
		catch (WCHAR* str) {
			OutputDebugInfo(L"Exception raised: %s", str);
		}
		
		pTagInfo->Release();
	}

	if (strIcon->Length() > 0)
		return TRUE;
	else
		return FALSE;
}

//-----------------------------------------------------------------------------

BOOL GetCoverArtFromFile(long nIndex, QString* strIcon)
{
	size_t nPos = 0;
	QString strFinal;
	
	std::wstring strRoot = Settings.strCoverArtRoot;
	std::wstring strTemplate = Settings.strCoverArtTemplate;
	
	if (!strIcon)
		return FALSE;
	
	//-----------------------------------------------------------------------------
	// Parse root
	// %CURRENT_DIR
	if (strRoot.substr(0, 12) == L"%CURRENT_DIR")
	{
		strFinal.SetUnicode(_GetFileFolder(nIndex));
	}
	else
	{
		strFinal.SetUnicode(strRoot.c_str());
	}

	
	//-----------------------------------------------------------------------------
	// Parse template
	// %A = Artist
	// %D = Album
	// %T = Track
	// %F = Filename with out ext
	// %E = Supported extensions (png, jpg, gif, bmp)

	// Find last . position
	nPos = strTemplate.find_last_of('.');
	if ((int)nPos < 1)
		return FALSE;

	// Parse template up to .
	for (std::wstring::size_type i = 0; i < nPos; i++)
	{
		if (strTemplate[i] == '%')
		{
			i++;
			switch (strTemplate[i])
			{
			case 'A' :
				strFinal.AppendUnicode(_GetInfo(opGetArtistName, nIndex));
				break;
			case 'D' :
				strFinal.AppendUnicode(_GetInfo(opGetAlbumName, nIndex));
				break;
			case 'F' :
				strFinal.AppendUnicode(_GetFilename(nIndex));
				break;
			case 'T' :
				strFinal.AppendUnicode(_GetInfo(opGetTrackName, nIndex));
				break;
			}
		}
		else
		{
			WCHAR w[2]; w[0] = strTemplate[i]; w[1] = 0;
			strFinal.AppendUnicode(w);
		}
	}

	// OutputDebugString(strFinal);
	
	//-----------------------------------------------------------------------------
	// Parse extension
	// *.png
	// *.jpg
	// *.gif
	// *.bmp
	static const int NUM_OF_EXT = 4;
	WCHAR* arrExtensions[NUM_OF_EXT];
	arrExtensions[0] = L"png";
	arrExtensions[1] = L"jpg";
	arrExtensions[2] = L"gif";
	arrExtensions[3] = L"bmp";

	strFinal.AppendUnicode(L".");

	if (strTemplate.substr(nPos+1, 2) == L"%E")
	{
		QString strFileTest;
		// Check if one of the files exist
		for (int i = 0; i < NUM_OF_EXT; i++)
		{
			strFileTest.SetUnicode(strFinal);
			strFileTest.AppendUnicode(arrExtensions[i]);
			HANDLE hFile = CreateFile(strFileTest, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
										NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE) {
				CloseHandle(hFile);
				strIcon->SetUnicode(strFileTest);
				break;
			}
		}
	}
	else
	{
		strFinal.AppendUnicode(strTemplate.substr(nPos+1).c_str());
		HANDLE hFile = CreateFile(strFinal, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hFile);
			strIcon->SetUnicode(strFinal);
		}		
	}
	if (strIcon->Length() > 0)
		return TRUE;
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------

QString _GetInfo(PluginServiceOp op, long nIndex)
{
	/*QString strReturn;
	WCHAR strUCS2[MAX_PATH];
	
	Service(op, strUCS2, MAX_PATH*sizeof(WCHAR), nIndex);
	strReturn.SetUnicode(strUCS2);
	return strReturn;*/

	WCHAR strUCS2[MAX_PATH];
	
	Service(op, strUCS2, MAX_PATH*sizeof(WCHAR), nIndex);
	return strUCS2;
}

//-----------------------------------------------------------------------------

QString _GetFilename(long nIndex)
{
	std::wstring strTmp;
	WCHAR strUCS2[MAX_PATH];
	
	Service(opGetPlaylistFile, strUCS2, MAX_PATH*sizeof(WCHAR), nIndex);
	strTmp = strUCS2;
	QString strRet = strTmp.substr(0, strTmp.find_last_of('.')).c_str();
	return strRet;
}

//-----------------------------------------------------------------------------

QString _GetFileFolder(long nIndex)
{
	QString strReturn;
	WCHAR  strUCS2[MAX_PATH];
	TCHAR  szPath[MAX_PATH] = {0};
	TCHAR* pPathEnd = NULL;

	Service(opGetPlaylistFile, strUCS2, MAX_PATH*sizeof(WCHAR), nIndex);

	GetFullPathNameW(strUCS2, MAX_PATH, szPath, &pPathEnd);
	*pPathEnd = '\0';
	
	strReturn.SetUnicode(szPath);
	return strReturn;
}


#endif // COVER_ART_H