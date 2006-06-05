#if !defined(AFX_CUSTOMCOMMAND_H__97A08577_9C33_45FB_BF31_AAD9B9419B4B__INCLUDED_)
#define AFX_CUSTOMCOMMAND_H__97A08577_9C33_45FB_BF31_AAD9B9419B4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomCommand.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustomCommand dialog

class CCustomCommand : public CDialog
{
// Construction
public:
	CCustomCommand(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCustomCommand)
	enum { IDD = IDD_CUSTOM };
	CSpinButtonCtrl	m_cRepeatSpin;
	BOOL	m_bSendOnce;
	UINT	m_iCommand;
	UINT	m_iRepeatCount;
	CString	m_strButton;
	CString	m_strCommandName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomCommand)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCustomCommand)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMCOMMAND_H__97A08577_9C33_45FB_BF31_AAD9B9419B4B__INCLUDED_)
