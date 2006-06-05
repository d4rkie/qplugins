// ConfigModify.cpp : implementation file
//

#include "stdafx.h"
#include "WinLIRC.h"
#include "CustomCommand.h"
#include "ConfigModify.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// #define NUMBER_OF_ITEMS 24 // located in .h
#define CASE_PLAY				0
#define CASE_STOP				1
#define CASE_PAUSE				2
#define CASE_NEXTTRACK			3
#define CASE_PREVIOUSTRACK		4
#define CASE_SEEKFORWARD		5
#define CASE_SEEKBACKWARD		6
#define CASE_VOLUP1				7
#define CASE_VOLDOWN1			8
#define CASE_VOLUP2				9
#define CASE_VOLDOWN2			10
#define CASE_VOLUP5				11
#define CASE_VOLDOWN5			12
#define CASE_VOLUP10			13
#define CASE_VOLDOWN10			14
#define CASE_MUTE				15
#define CASE_REPEATALL			16
#define CASE_REPEAT1			17
#define CASE_SHUFFLE			18
#define CASE_QUIT				19
#define CASE_SHUTDOWNPC			20
#define CASE_SUSPENDPC			21
#define CASE_HIBERNATEPC		22

//#define CASE_CUSTOM				23

/////////////////////////////////////////////////////////////////////////////
// CConfigModify dialog


CConfigModify::CConfigModify(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigModify::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigModify)
	//}}AFX_DATA_INIT

	m_arrListItems[0]	= "Play";
	m_arrListItems[1]	= "Stop";
	m_arrListItems[2]	= "Pause";
	m_arrListItems[3]	= "Next track";
	m_arrListItems[4]	= "Previous Track";
	m_arrListItems[5]	= "Seek Forward";
	m_arrListItems[6]	= "Seek Backward";
	m_arrListItems[7]	= "Volume Up (+1)";
	m_arrListItems[8]	= "Volume Down (-1)";
	m_arrListItems[9]	= "Volume Up (+2)";
	m_arrListItems[10]	= "Volume Down (-2)";
	m_arrListItems[11]	= "Volume Up (+5)";
	m_arrListItems[12]	= "Volume Down (-5)";
	m_arrListItems[13]	= "Volume Up (+10)";
	m_arrListItems[14]	= "Volume Down (-10)";
	m_arrListItems[15]	= "Mute";
	m_arrListItems[16]	= "Repeat all";
	m_arrListItems[17]	= "Repeat 1";
	m_arrListItems[18]	= "Shuffle";
	m_arrListItems[19]	= "Quit QCD";
	m_arrListItems[20]	= "Shutdown PC";
	m_arrListItems[21]	= "Suspend PC";
	m_arrListItems[22]	= "Hibernate PC";
	
	m_arrListItems[23]	= "Custom...";		// Custom is assumed NUMBER_OF_ITEMS - 1 !
}


void CConfigModify::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigModify)
	DDX_Control(pDX, IDC_MODIFY_BUTTON, m_cButton);
	DDX_Control(pDX, IDC_MODIFY_ACTION, m_cAction);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigModify, CDialog)
	//{{AFX_MSG_MAP(CConfigModify)
	ON_CBN_SELENDOK(IDC_MODIFY_ACTION, OnSelendokModifyAction)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigModify message handlers

void CConfigModify::OnSelendokModifyAction() 
{
	if (m_cAction.GetCurSel() == NUMBER_OF_ITEMS - 1) { // Custom ... selected
		CString strTemp;
		CCustomCommand dlg;		

		m_cButton.GetWindowText(strTemp);

		dlg.m_strButton = strTemp;

		if (IDOK == dlg.DoModal()) {
			Command* pCmd = (Command*)pArrCommands->GetAt(m_iCommandIndex);

			if (pCmd && !dlg.m_strButton.IsEmpty() && !dlg.m_strCommandName.IsEmpty()) {
				pCmd->bCustom		= TRUE;
				pCmd->strCommand	= dlg.m_strCommandName;
				pCmd->strButton		= dlg.m_strButton;

				pCmd->bSendOnce		= dlg.m_bSendOnce;
				pCmd->iCommand		= dlg.m_iCommand;
				pCmd->iRepeatCount	= dlg.m_iRepeatCount;
			}
			CDialog::OnOK();
		}
	}
}

void CConfigModify::OnOK() 
{
	CString strTemp;

	// Update the Command struct
	Command* pCmd = (Command*)pArrCommands->GetAt(m_iCommandIndex);

	ASSERT(pCmd);

	if (pCmd) {
		m_cButton.GetWindowText(strTemp);
		pCmd->strButton = strTemp;

		DecodeAction(pCmd);
	}		
	
	CDialog::OnOK();
}

int CConfigModify::DoModal() 
{
	return CDialog::DoModal();
}

void CConfigModify::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	// Add all the items
	for (int i = 0; i < NUMBER_OF_ITEMS; i++)
		m_cAction.AddString(m_arrListItems[i]);

	// Select the right one
	if (((Command*)pArrCommands->GetAt(m_iCommandIndex))->strCommand.Compare(_T("Custom")) == 0)
		m_cAction.SetCurSel(NUMBER_OF_ITEMS - 1);
	else {
		m_cButton.SetWindowText(((Command*)pArrCommands->GetAt(m_iCommandIndex))->strButton);
		INT iIndex = m_cAction.FindStringExact(1, ((Command*)pArrCommands->GetAt(m_iCommandIndex))->strCommand);
		m_cAction.SetCurSel(iIndex);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Decodes all the actions to the right WM_COMMAND
/////////////////////////////////////////////////////////////////////////////
void CConfigModify::DecodeAction(Command* pCmd)
{
	BOOL bSendOnce;
	INT iRepeatCount = 0, iCommand = 0;

	INT iCurSel = m_cAction.GetCurSel();
	
	if (iCurSel > -1 && iCurSel < NUMBER_OF_ITEMS)
	{
		switch (iCurSel)
		{
		case CASE_PLAY :	// Play
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_PLAY;
			break;

		case CASE_STOP :	// Stop			
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_STOP;
			break;

		case CASE_PAUSE :	// Pause
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_PAUSE;
			break;

		case CASE_NEXTTRACK :	// Next Track
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_TRKFWD;
			break;

		case CASE_PREVIOUSTRACK :	// Previous Track
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_TRKBWD;
			break;

		case CASE_SEEKFORWARD :	// Seek Forward
			bSendOnce		= false;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_FWD5;
			break;

		case CASE_SEEKBACKWARD :	// Seek Backward
			bSendOnce		= false;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_BWD5;
			break;

		case CASE_VOLUP1 :	// Volume Up (+1)
			bSendOnce		= false;
			iRepeatCount	= 1;
			iCommand		= 0;
			break;

		case CASE_VOLDOWN1 :// Volume Down (-1)
			bSendOnce		= false;
			iRepeatCount	= 1;
			iCommand		= 0;
			break;

		case CASE_VOLUP2 :	// Volume Up (+2)
			bSendOnce		= false;
			iRepeatCount	= 2;
			iCommand		= 0;
			break;

		case CASE_VOLDOWN2 :	// Volume Down (-2)
			bSendOnce		= false;
			iRepeatCount	= 2;
			iCommand		= 0;
			break;

		case CASE_VOLUP5 :	// Volume Up (+5)
			bSendOnce		= false;
			iRepeatCount	= 5;
			iCommand		= 0;
			break;

		case CASE_VOLDOWN5 :// Volume Down (-5)
			bSendOnce		= false;
			iRepeatCount	= 5;
			iCommand		= 0;
			break;

		case CASE_VOLUP10 :	// Volume Up (+10)
			bSendOnce		= true;
			iRepeatCount	= 10;
			iCommand		= 0;
			break;

		case CASE_VOLDOWN10 :	// Volume Down (-10)
			bSendOnce		= true;
			iRepeatCount	= 10;
			iCommand		= 0;
			break;

		case CASE_MUTE :	// Mute
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= 0;
			break;

		case CASE_REPEATALL :	// Repeat all
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_REPEATALL;
			break;

		case CASE_REPEAT1 :	// Repeat 1
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_REPEATTRACK;
			break;

		case CASE_SHUFFLE :	// Shuffle
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= QCD_COMMAND_SHUFFLE;
			break;

		case CASE_QUIT :	// Quit QCD
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= 40003;	// ID_MENU_EXIT
			break;

		case CASE_SHUTDOWNPC :	// Shutdown PC
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= 0;
			break;

		case CASE_SUSPENDPC :	// Suspend PC
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= 0;
			break;

		case CASE_HIBERNATEPC :	// Hibernate PC
			bSendOnce		= true;
			iRepeatCount	= 1;
			iCommand		= 0;
			break;

		}

		pCmd->strCommand	= m_arrListItems[iCurSel];
		pCmd->bSendOnce		= bSendOnce;
		pCmd->iCommand		= iCommand;
		pCmd->iRepeatCount	= iRepeatCount;
	}

	/*CString strD;
	strD.Format("%d", m_cAction.GetCurSel());
	AfxMessageBox(strD);*/
}
