// CfgFile.h: interface for the CCfgFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CFGFILE_H__63C3A9BF_7D79_4035_8C1E_E8773DB192C2__INCLUDED_)
#define AFX_CFGFILE_H__63C3A9BF_7D79_4035_8C1E_E8773DB192C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCfgFile  
{
public:
	CCfgFile(CString strFile);
	virtual ~CCfgFile();

	VOID Read();
	VOID Save();
	VOID LoadDefault();
private:
	CString m_strFile;
};

#endif // !defined(AFX_CFGFILE_H__63C3A9BF_7D79_4035_8C1E_E8773DB192C2__INCLUDED_)
