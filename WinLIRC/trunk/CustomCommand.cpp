// CustomCommand.cpp : implementation file
//

#include "stdafx.h"
#include "WinLIRC.h"
#include "CustomCommand.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomCommand dialog


CCustomCommand::CCustomCommand(CWnd* pParent /*=NULL*/)
	: CDialog(CCustomCommand::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCustomCommand)
	m_bSendOnce = FALSE;
	m_iCommand = 0;
	m_iRepeatCount = 0;
	m_strButton = _T("");
	m_strCommandName = _T("");
	//}}AFX_DATA_INIT
}


void CCustomCommand::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomCommand)
	DDX_Control(pDX, IDC_CUSTOM_REPEATSPIN, m_cRepeatSpin);
	DDX_Check(pDX, IDC_CUSTOM_SENDONCE, m_bSendOnce);
	DDX_Text(pDX, IDC_CUSTOM_COMMAND, m_iCommand);
	DDV_MinMaxUInt(pDX, m_iCommand, 0, 65535);
	DDX_Text(pDX, IDC_CUSTOM_REPEATCOUNT, m_iRepeatCount);
	DDV_MinMaxUInt(pDX, m_iRepeatCount, 0, 65535);
	DDX_Text(pDX, IDC_CUSTOM_BUTTON_NAME, m_strButton);
	DDV_MaxChars(pDX, m_strButton, 256);
	DDX_Text(pDX, IDC_CUSTOM_COMMAND_NAME, m_strCommandName);
	DDV_MaxChars(pDX, m_strCommandName, 256);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomCommand, CDialog)
	//{{AFX_MSG_MAP(CCustomCommand)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomCommand message handlers

void CCustomCommand::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	m_cRepeatSpin.SetRange(1, 100);
	if (m_iRepeatCount < 1)
		m_cRepeatSpin.SetPos(1);
}
