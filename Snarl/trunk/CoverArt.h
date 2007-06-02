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
	g_strTempPath.assign(_wgetenv(L"TEMP"));
	g_strTempPath.append(L"\\");
}

//-----------------------------------------------------------------------------

void CoverArtShutdown()
{
	QString strTmp;
	// Clean up the files we created
	for (std::deque<QString*>::iterator aiIter = g_listTempFiles.begin(); aiIter != g_listTempFiles.end( ); aiIter++)
	{
		strTmp = g_strTempPath;
		strTmp.append(**aiIter);
		DeleteFile(strTmp);
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
		try {
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
				strFilename->append(_GetInfo(opGetArtistName, nIndex));
				strFilename->append(L" - ");
				strFilename->append(_GetInfo(opGetAlbumName, nIndex));
				strFilename->append(L".");
				
				if (wcscmp(pArtwork->pszMimeType, L"image/jpeg") == 0)
					strFilename->append(L"jpg");

				QString strFullPath;
				strFullPath = g_strTempPath;
				strFullPath.append(*strFilename);

				// Output to temp file
				HANDLE hFile = CreateFile(strFullPath.GetUnicode(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE) {
					DWORD nWritten = 0;
					if (WriteFile(hFile, pArtwork->pbData, pArtwork->dwDataLen, &nWritten, NULL)) {
						strIcon->assign(strFullPath);
						g_listTempFiles.push_back(strFilename);
					}
					else
						delete strFilename;
					CloseHandle(hFile);
				}
				else {
					DWORD nErr = GetLastError();
					
					if (nErr == ERROR_FILE_EXISTS)
						strIcon->assign(strFullPath);
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

	if (strIcon->length() > 0)
		return TRUE;
	else
		return FALSE;
}

//-----------------------------------------------------------------------------

BOOL GetCoverArtFromFile(long nIndex, QString* strIcon)
{
	QString::size_type nPos = 0;
	QString strFinal;
	
	QString strRoot = Settings.strCoverArtRoot;
	QString strTemplate = Settings.strCoverArtTemplate;
	
	if (!strIcon)
		return FALSE;
	
	// Do approx. memory reservation up front
	strFinal.reserve(Settings.strCoverArtRoot.size() + strTemplate.size() + 32);

	//-----------------------------------------------------------------------------
	// Parse root
	// %CURRENT_DIR
	if (strRoot.substr(0, 12) == L"%CURRENT_DIR")
	{
		strFinal.assign(_GetFileFolder(nIndex));
	}
	else
	{
		strFinal.assign(strRoot);
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
	for (QString::size_type i = 0; i < nPos; i++)
	{
		if (strTemplate[i] == '%')
		{
			i++;
			switch (strTemplate[i])
			{
			case 'A' :
				strFinal.append(_GetInfo(opGetArtistName, nIndex));
				break;
			case 'D' :
				strFinal.append(_GetInfo(opGetAlbumName, nIndex));
				break;
			case 'F' :
				strFinal.append(_GetFilename(nIndex));
				break;
			case 'T' :
				strFinal.append(_GetInfo(opGetTrackName, nIndex));
				break;
			}
		}
		else
			strFinal.push_back(strTemplate[i]);
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

	strFinal.push_back(L'.');

	if (strTemplate.substr(nPos+1, 2) == L"%E")
	{
		QString strFileTest;
		// Check if one of the files exist
		for (int i = 0; i < NUM_OF_EXT; i++)
		{
			strFileTest = strFinal;
			strFileTest.append(arrExtensions[i]);
			HANDLE hFile = CreateFile(strFileTest, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
										NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE) {
				CloseHandle(hFile);
				strIcon->assign(strFileTest);
				break;
			}
		}
	}
	else
	{
		strFinal.append(strTemplate.substr(nPos+1));
		HANDLE hFile = CreateFile(strFinal, 0, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hFile);
			strIcon->assign(strFinal);
		}		
	}
	if (strIcon->length() > 0)
		return TRUE;
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------

QString _GetInfo(PluginServiceOp op, long nIndex)
{
	QString strReturn;
	WCHAR strUCS2[MAX_PATH];
	
	Service(op, strUCS2, MAX_PATH*sizeof(WCHAR), nIndex);
	strReturn = strUCS2;
	return strReturn;
}

//-----------------------------------------------------------------------------

QString _GetFilename(long nIndex)
{
	QString strReturn;
	WCHAR strUCS2[MAX_PATH];
	
	Service(opGetPlaylistFile, strUCS2, MAX_PATH*sizeof(WCHAR), nIndex);
	strReturn = strUCS2;
	return strReturn.substr(0, strReturn.find_last_of('.')).c_str();
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