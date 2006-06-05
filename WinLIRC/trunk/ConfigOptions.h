#if !defined(AFX_CONFIGOPTIONS_H__B6A471FF_98D2_4EF5_9151_24E66154C37C__INCLUDED_)
#define AFX_CONFIGOPTIONS_H__B6A471FF_98D2_4EF5_9151_24E66154C37C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigOptions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigOptions dialog

class CConfigOptions : public CDialog
{
// Construction
public:
	CConfigOptions(CWnd* pParent = NULL);   // standard constructor
	void BrowseForWinLIRC();

// Dialog Data
	//{{AFX_DATA(CConfigOptions)
	enum { IDD = IDD_OPTIONS };
	CEdit	m_cPath;
	BOOL	m_bDebugMessages;
	BOOL	m_bShowErrorMessages;
	BOOL	m_bLanchWinLIRC;
	BOOL	m_bConnectOnStart;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigOptions)
	virtual void OnOK();
	afx_msg void OnOptionsBrowseBtn();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGOPTIONS_H__B6A471FF_98D2_4EF5_9151_24E66154C37C__INCLUDED_)
