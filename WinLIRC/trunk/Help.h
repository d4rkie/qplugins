#if !defined(AFX_HELP_H__3BB964C8_DC39_4B2F_8F40_1ED8EC5920F6__INCLUDED_)
#define AFX_HELP_H__3BB964C8_DC39_4B2F_8F40_1ED8EC5920F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Help.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHelp dialog

class CHelp : public CDialog
{
// Construction
public:
	CHelp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHelp)
	enum { IDD = IDD_HELP };
	CString	m_strEdit1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHelp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHelp)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HELP_H__3BB964C8_DC39_4B2F_8F40_1ED8EC5920F6__INCLUDED_)
