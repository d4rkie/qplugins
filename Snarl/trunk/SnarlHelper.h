#ifndef SnarlHelpter_H
#define SnarlHelpter_H

#include "QCDGeneralDLL.h"

void StartSnarl();
void CloseSnarl();
BOOL GetSnarlPath(QString* strPath, QString* strDir);



void StartSnarl()
{
	HWND hWnd = NULL;
	QString strPath, strDirectory;

	// Check if already running
	if (IsWindow(SnarlInterface::GetSnarlWindow()))
		return;

	// Get path from registry
	if (!GetSnarlPath(&strPath, &strDirectory))
		return;

	INT iError = (UINT)ShellExecute(NULL, L"open", strPath, NULL, strDirectory, SW_SHOWDEFAULT);
	if (iError <= 32) {
		QString strError;
		switch (iError) {
			case 0 :
				strError = L"The operating system is out of memory or resources.";
				break;
			case ERROR_FILE_NOT_FOUND :
				strError = L"The specified file was not found.";
				break;
			case ERROR_PATH_NOT_FOUND :
				strError = L"The specified path was not found.";
				break;
			case SE_ERR_ACCESSDENIED : 
				strError = L"The operating system denied access to the specified file.";
			default :
				strError = L"Unknown error. Error number: %d";
				break;
		}
		OutputDebugInfo(L"StartSnarl() : ShellExecute error : %s", strError.GetUnicode(), iError);
	}
}

//-----------------------------------------------------------------------------

void CloseSnarl()
{
	int nCloseMsg = 0; 
	if (g_nSnarlVersion == 0) {
		SnarlInterface snarl;
		g_nSnarlVersion = snarl.GetVersionEx();
	}

	nCloseMsg = (g_nSnarlVersion < 39) ? WM_USER + 81 : WM_CLOSE; // This is fixed with 2.06 aka 38 release 6, which we can't test for yet.

	DWORD nReturn = 0;
	HWND hWnd = SnarlInterface::GetSnarlWindow();
	if (IsWindow(hWnd))
		PostMessage(hWnd, nCloseMsg, 0, 0);
}

//-----------------------------------------------------------------------------

BOOL GetSnarlPath(QString* strPath, QString* strDir)
{
	const static WCHAR* hSubKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Snarl.exe";
	DWORD nPathSize = MAX_PATH*sizeof(WCHAR);
	WCHAR strTemp[MAX_PATH] = {0};
	DWORD dwBufLen = MAX_PATH;
	
	
	//LONG nRet = RegGetValue(HKEY_LOCAL_MACHINE, hSubKey, NULL, RRF_RT_REG_SZ, NULL, strTemp, &nPathSize);
	HKEY hKey = 0;
	LONG nRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, hSubKey, 0, KEY_QUERY_VALUE, &hKey);
	if (nRet == ERROR_SUCCESS)
	{
		LONG lRet = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)strTemp, &dwBufLen);
		RegCloseKey(hKey);

		if((lRet != ERROR_SUCCESS) || (dwBufLen >= MAX_PATH))
			return FALSE;

		// Set strPath
		strPath->SetUnicode(strTemp);

		// Set strDir
		std::wstring str = strTemp;
		int nPos = str.rfind(L"\\");
		strDir->SetUnicode(str.substr(0, nPos+1).c_str());

		return TRUE;
	}
	return FALSE;
}

#endif // SnarlHelpter_H