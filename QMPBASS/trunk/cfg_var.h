#pragma once

#include <windows.h>

//-- class cfg_int
class cfg_int
{
private:
	LPCTSTR m_lpAppName;
	LPCTSTR m_lpKeyName;
	INT m_nValue;
public:
	cfg_int(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault);
	~cfg_int(void);
public:
	void load(LPCTSTR lpFileName);
	BOOL save(LPCTSTR lpFileName);
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
	cfg_string(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, DWORD nMaxSize);
	~cfg_string(void);
public:
	void load(LPCTSTR lpFileName);
	BOOL save(LPCTSTR lpFileName);
public:
	inline operator LPCTSTR (void) const { return m_lpValue; }
	inline bool is_empty() { return !m_lpValue || !m_lpValue[0]; }
};

