#pragma once

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

//-- class cfg_int
class cfg_int
{
private:
	LPCTSTR m_lpAppName;
	LPCTSTR m_lpKeyName;
	INT m_nValue;
public:
	cfg_int(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault)
		: m_lpAppName(lpAppName)
		, m_lpKeyName(lpKeyName)
		, m_nValue(nDefault)
	{}
	~cfg_int(void) {}
public:
	void load(LPCTSTR lpFileName)
	{
		m_nValue = GetPrivateProfileInt(m_lpAppName, m_lpKeyName, m_nValue, lpFileName);
	}
	BOOL save(LPCTSTR lpFileName)
	{
		TCHAR str[20];
		_stprintf(str, "%d", m_nValue);
		return WritePrivateProfileString(m_lpAppName, m_lpKeyName, str, lpFileName);
	}
public:
	inline operator int() const { return m_nValue; }
	inline long operator=(INT nValue)
	{
		m_nValue = nValue;
		return nValue;
	}
	inline long operator=(const cfg_int & cfgValue)
	{
		m_nValue = cfgValue;
		return cfgValue;
	}
};

//-- class cfg_string
class cfg_string
{
private:
	LPCTSTR m_lpAppName;
	LPCTSTR m_lpKeyName;
	LPTSTR m_lpValue;
	UINT m_nSize;
public:
	cfg_string(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, DWORD nMaxSize)
		: m_lpAppName(lpAppName)
		, m_lpKeyName(lpKeyName)
		, m_nSize(nMaxSize)
	{
		if (m_nSize)
			m_lpValue = new TCHAR[m_nSize];

		if (m_lpValue)
			lstrcpy(m_lpValue, lpDefault);
	}
	~cfg_string(void)
	{
		if (m_lpValue) {
			delete [] m_lpValue;
			m_lpValue = NULL;
		}
	}
public:
	void load(LPCTSTR lpFileName)
	{
		LPTSTR lpTemp = _tcsdup(m_lpValue);

		GetPrivateProfileString(m_lpAppName, m_lpKeyName, lpTemp, m_lpValue, m_nSize, lpFileName);

		free(lpTemp);
	}
	BOOL save(LPCTSTR lpFileName)
	{
		return WritePrivateProfileString(m_lpAppName, m_lpKeyName, m_lpValue, lpFileName);
	}
public:
	inline operator LPCTSTR (void) const { return m_lpValue; }
	inline bool is_empty() { return !m_lpValue || !m_lpValue[0]; }
};

