#include <TCHAR.h>
#include <Windows.h>

#include "QCDGeneralDLL.h"
#include "WMP_Reg.h"


static const WCHAR*  REGKEY_MSN    = L"Software\\Microsoft\\MSNMessenger";
static const WCHAR*  REGKEY_WMP    = L"Software\\Microsoft\\Active Setup\\Installed Components\\{6BF52A52-394A-11d3-B153-00C04F79FAA6}";

//-----------------------------------------------------------------------------
// RegDB functions for faking WMP 10
//-----------------------------------------------------------------------------
void RegDB_Fix(BOOL bFix)
{
	static BOOL bIsFixed = FALSE;

	if (bFix) {
		if (RegDB_GetWMPVersion() < 9) {
			RegDB_Insert();
			settings.bWMPIsFaked = true;
		}
	}
	else {
			RegDB_Clean();
			settings.bWMPIsFaked = false;
	}
}

//-----------------------------------------------------------------------------

void RegDB_Insert()
{
	HKEY hKey = NULL;
	DWORD dwDis = NULL;
	LPTSTR lpClass = _T("");
	
	SECURITY_ATTRIBUTES lpSecurityAtt;
	lpSecurityAtt.nLength = sizeof(LPSECURITY_ATTRIBUTES);
	lpSecurityAtt.lpSecurityDescriptor = NULL;
	lpSecurityAtt.bInheritHandle = TRUE;

	if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGKEY_WMP, 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, &lpSecurityAtt, &hKey, &dwDis))
	{
		DWORD nValue;

		RegSetValueEx(hKey, _T(""),            0, REG_SZ,    (CONST BYTE*)_T("Microsoft Windows Media Player"), 31*sizeof(_TCHAR));
		RegSetValueEx(hKey, _T("ComponentID"), 0, REG_SZ,    (CONST BYTE*)_T("Microsoft Windows Media Player"), 31*sizeof(_TCHAR));
		RegSetValueEx(hKey, _T("Locale"),      0, REG_SZ,    (CONST BYTE*)_T("EN"), 3*sizeof(_TCHAR));
		RegSetValueEx(hKey, _T("StubPath"),    0, REG_SZ,    (CONST BYTE*)_T(""), 1*sizeof(_TCHAR));
		RegSetValueEx(hKey, _T("Version"),     0, REG_SZ,    (CONST BYTE*)_T("10,0,0,3646"), 12*sizeof(_TCHAR));

		nValue = 2; RegSetValueEx(hKey, _T("DontAsk"),     0, REG_DWORD, (CONST BYTE*)&nValue, 4);
		nValue = 1; RegSetValueEx(hKey, _T("IsInstalled"), 0, REG_DWORD, (CONST BYTE*)&nValue, 4);

		RegCloseKey(hKey);
	}
}

//-----------------------------------------------------------------------------

void RegDB_Clean()
{
	RegDeleteKey(HKEY_LOCAL_MACHINE, REGKEY_WMP);
}

//-----------------------------------------------------------------------------

INT RegDB_GetWMPVersion()
{
	UINT nVersion = 0;
	HKEY hKey = NULL;

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_WMP, 0,  KEY_READ, &hKey)) {
		BYTE strBuff[64];
		DWORD nBuffSize = 64;
		DWORD nType = 0;
		_TCHAR* pPos = NULL;

		RegQueryValueEx(hKey, L"Version", NULL, &nType, strBuff, &nBuffSize);		
		RegCloseKey(hKey);

		if (nBuffSize == 64) // Might not be 0 terminated
			strBuff[63] = 0;

		if (nBuffSize > 0) {
			pPos = wcsstr((WCHAR*)strBuff, L",");
			
			if (pPos) {
				*pPos = 0;
				nVersion = _ttoi((WCHAR*)strBuff);
			}
		}
	}
	return nVersion;
}

//-----------------------------------------------------------------------------
// Returns MSNMessenger build version
//-----------------------------------------------------------------------------
INT RegDB_GetMSNBuild()
{
	UINT nBuild = 0;
	HKEY hKey = NULL;

	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_MSN, 0,  KEY_READ, &hKey)) {
		WCHAR strBuff[64];
		DWORD nBuffSize = 64;
		DWORD nType = 0;
		WCHAR* pPos = NULL;

		RegQueryValueEx(hKey, L"AppCompatCanary", NULL, &nType, (LPBYTE)strBuff, &nBuffSize);
		RegCloseKey(hKey);

		if (nBuffSize == 64) // Might not be 0 terminated
			strBuff[63] = L'\0';

		if (nBuffSize > 0) {
			pPos = wcsrchr(strBuff, L'.');
			
			if (pPos) {
				pPos++;
				nBuild = _wtoi(pPos);
			}
		}
	}
	return nBuild;
}