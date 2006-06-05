#if !defined(AFX_ConfigModify_H__358A7F0B_8BB7_4448_BA7E_3AAE132370C9__INCLUDED_)
#define AFX_ConfigModify_H__358A7F0B_8BB7_4448_BA7E_3AAE132370C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NUMBER_OF_ITEMS 24

// ConfigModify.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigModify dialog

class CConfigModify : public CDialog
{
// Construction
public:
	CConfigModify(CWnd* pParent = NULL);   // standard constructor

	INT m_iCommandIndex;

// Dialog Data
	//{{AFX_DATA(CConfigModify)
	enum { IDD = IDD_MODIFY };
	CEdit	m_cButton;
	CComboBox	m_cAction;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigModify)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigModify)
	afx_msg void OnSelendokModifyAction();
	virtual void OnOK();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void DecodeAction(Command* pCmd);	// Helper function
	CString m_arrListItems[NUMBER_OF_ITEMS];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ConfigModify_H__358A7F0B_8BB7_4448_BA7E_3AAE132370C9__INCLUDED_)
