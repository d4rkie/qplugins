// Config.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Config.h"
#include "ConfigModify.h"
#include "ConfigOptions.h"
#include "CustomCommand.h"
#include "Help.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Init all static members - Must be done at file level
//////////////////////////////////////////////////////////////////////

Settings* CConfig::m_pSettings			= 0;
HWND CConfig::m_hwndDlg					= 0;
HWND CConfig::m_hwndConnStatus			= 0;
HWND CConfig::m_hwndWinLIRCStatus		= 0;
HWND CConfig::m_hwndReconnectBtn		= 0;
HWND CConfig::m_hwndLaunchWinLIRCBtn	= 0;
HWND CConfig::m_hwndList				= 0;

// Strings

const _TCHAR* CConfig::Status_Connected		= _T("Connected!");
const _TCHAR* CConfig::Status_NotConnected	= _T("Not connected!");
const _TCHAR* CConfig::Status_Running		= _T("WinLIRC is running!");
const _TCHAR* CConfig::Status_NotRunning	= _T("WinLIRC is not running!");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{
	if (!hInstance)
		MessageBox(hwndPlayer, "Error: !hInstance", "CConfig::CConfig()", MB_OK);

	PluginPrefPage pPPP;
	pPPP.struct_size	= sizeof(PluginPrefPage);
	pPPP.hModule		= hInstance;
	pPPP.lpTemplate		= MAKEINTRESOURCEW(IDD_CONFIG);
    pPPP.lpDialogFunc	= DialogCallback;
	pPPP.lpDisplayText	= L"WinLIRC";
	pPPP.nCategory		= PREFPAGE_CATEGORY_PLUGINS;
	
	m_iPageID = QCDCallbacks->Service(opSetPluginPage, &pPPP, 0, 0);
	if (m_iPageID == 0)
		MessageBox(hwndPlayer, _T("Init failed when trying to insert preference page."), _T("Init Error!"), MB_OK);

}

CConfig::~CConfig()
{

}

//----------------------------------------------------------------------------
// Init functions
//----------------------------------------------------------------------------
void CConfig::DialogInit(HWND hwndDlg)
{
	m_hwndDlg				= hwndDlg;
	m_hwndConnStatus		= GetDlgItem(hwndDlg, IDC_CONNECTION_STATUS);
	m_hwndWinLIRCStatus		= GetDlgItem(hwndDlg, IDC_WinLIRC_STATUS);
	m_hwndReconnectBtn		= GetDlgItem(hwndDlg, IDC_RECONNECT_BTN);
	m_hwndLaunchWinLIRCBtn	= GetDlgItem(hwndDlg, IDC_LAUNCH_WINLIRC_BTN);
	m_hwndList				= GetDlgItem(hwndDlg, IDC_LIST);

	if (m_hwndConnStatus && m_hwndWinLIRCStatus && m_hwndReconnectBtn && m_hwndLaunchWinLIRCBtn && m_hwndList) {
		// Connection status
		if (!m_pSettings->bConnected) {
			EnableWindow(m_hwndReconnectBtn, TRUE);
			SetWindowText(m_hwndConnStatus, Status_NotConnected);
		}
		else {
			EnableWindow(m_hwndReconnectBtn, FALSE);
			SetWindowText(m_hwndConnStatus, Status_Connected);
		}
		// WinLIRC running status
		if (IsWinLIRCRunning()) {
			EnableWindow(m_hwndLaunchWinLIRCBtn, FALSE);
			SetWindowText(m_hwndWinLIRCStatus, Status_Running);
		}
		else {
			EnableWindow(m_hwndLaunchWinLIRCBtn, TRUE);
			SetWindowText(m_hwndWinLIRCStatus, Status_NotRunning);
		}

		// List
		LVCOLUMN lvColumn;
		lvColumn.mask		= LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt		= LVCFMT_LEFT;

		lvColumn.cx			= 165;
		lvColumn.pszText	= "Button";
		ListView_InsertColumn(m_hwndList, 0, (LPARAM)&lvColumn);

		lvColumn.cx			= 220;
		lvColumn.pszText	= "Command";
		ListView_InsertColumn(m_hwndList, 1, (LPARAM)&lvColumn);

		// Insert all the items in the list
		LVITEM lvItem;
		lvItem.mask		= LVIF_TEXT | LVIF_PARAM;
		lvItem.iSubItem	= 0;
		
		for (int i = 0; i < pArrCommands->GetSize(); i++)
		{
			LPCTSTR pStrButton	= ((Command*)pArrCommands->GetAt(i))->strButton;
			LPCTSTR pStrCommand	= ((Command*)pArrCommands->GetAt(i))->strCommand;

			lvItem.iItem	= i;
			lvItem.pszText	= const_cast<LPTSTR>(pStrButton);
			lvItem.lParam	= i;

			ListView_InsertItem(m_hwndList, &lvItem);
			ListView_SetItemText(m_hwndList, i, 1, const_cast<LPTSTR>(pStrCommand));
		}
	}
	else
		MessageBox(hwndPlayer, "FillList() error", "Error", MB_OK);
}

//----------------------------------------------------------------------------
// Events
//----------------------------------------------------------------------------

void CConfig::OnAddBtn()
{
	INT iCommandIndex = 0, iListIndex = 0;

	// Add pArrCommand item
	Command* pCmd = new Command;
	iCommandIndex = pArrCommands->Add(pCmd);
	pCmd->bCustom  = FALSE;

	// Add list item
	iListIndex = ListView_GetItemCount(m_hwndList);
	LVITEM lvItem;
	lvItem.mask		= LVIF_TEXT | LVIF_PARAM;
	lvItem.iSubItem	= 0;
	lvItem.iItem	= iListIndex;
	lvItem.pszText	= _T("New item");
	lvItem.lParam	= pArrCommands->GetUpperBound();
	ListView_InsertItem(m_hwndList, &lvItem);
	ListView_SetItemText(m_hwndList, iListIndex, 1, _T("New item"););

	// Open modify dialog that takes care of rest
	CConfigModify dlg;
	dlg.m_iCommandIndex = iCommandIndex;

	if (IDOK == dlg.DoModal()) {	// Save
		// Update list
		if (!pCmd->strButton.IsEmpty()) {
			LPCTSTR pStr = pCmd->strButton;
			ListView_SetItemText(m_hwndList, iListIndex, 0, const_cast<LPTSTR>(pStr));
			pStr = pCmd->strCommand;
			ListView_SetItemText(m_hwndList, iListIndex, 1, const_cast<LPTSTR>(pStr));
			return;	// Success
		}
	}
	// IDCANCEL or strButton not set. Delete the new item

	// Select new item
	SetFocus(m_hwndList);
	ListView_SetItemState(m_hwndList, iListIndex, LVIS_SELECTED, LVIS_SELECTED);
	OnRemoveBtn();
}

void CConfig::OnRemoveBtn()
{
	INT iPos = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);

	if (iPos >= 0) {
		// Remove and DELETE !! the pArrCommand item
		delete (Command*)pArrCommands->GetAt(iPos);
		pArrCommands->RemoveAt(iPos);

		// Remove the list item
		ListView_DeleteItem(m_hwndList, iPos);
	}
}

void CConfig::OnModifyBtn()
{
	Command* pCmd;
	INT iPos = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
	
	if (iPos >= 0) {
		pCmd = (Command*)pArrCommands->GetAt(iPos);
		// Is custom?
		if (pCmd->bCustom) {
			CCustomCommand dlg;

			dlg.m_strButton			= pCmd->strButton;
			dlg.m_strCommandName	= pCmd->strCommand;
			dlg.m_bSendOnce			= pCmd->bSendOnce;
			dlg.m_iCommand			= pCmd->iCommand;
			dlg.m_iRepeatCount		= pCmd->iRepeatCount;

			if (IDOK == dlg.DoModal()) {
				if (!dlg.m_strButton.IsEmpty() && !dlg.m_strCommandName.IsEmpty()) {
					pCmd->strCommand	= dlg.m_strCommandName;
					pCmd->strButton		= dlg.m_strButton;
					pCmd->bSendOnce		= dlg.m_bSendOnce;
					pCmd->iCommand		= dlg.m_iCommand;
					pCmd->iRepeatCount	= dlg.m_iRepeatCount;
				}
			}
		}
		else {	// Normal
			CConfigModify dlg;
			dlg.m_iCommandIndex = iPos;

			if (IDOK == dlg.DoModal()) {	// Save
				pCmd = (Command*)pArrCommands->GetAt(iPos);
				// Update list
				LPCTSTR pStr = pCmd->strButton;
				ListView_SetItemText(m_hwndList, iPos, 0, const_cast<LPTSTR>(pStr));
				pStr = pCmd->strCommand;
				ListView_SetItemText(m_hwndList, iPos, 1, const_cast<LPTSTR>(pStr));
			}
		}
	}
}

void CConfig::OnOptionsBtn()
{
	CConfigOptions dlg;
	dlg.m_bDebugMessages = m_pSettings->bDebug;
	dlg.m_bShowErrorMessages = m_pSettings->bShowErrorMessages;

	if (IDOK == dlg.DoModal()) {	// Save
		m_pSettings->bDebug = dlg.m_bDebugMessages;
		m_pSettings->bShowErrorMessages = dlg.m_bShowErrorMessages;
	}
}

void CConfig::OnReconnectBtn()
{
	Connect();

	if (!m_pSettings->bConnected) {
		EnableWindow(m_hwndReconnectBtn, TRUE);
		SetWindowText(m_hwndConnStatus, Status_NotConnected);
	}
	else {
		EnableWindow(m_hwndReconnectBtn, FALSE);
		SetWindowText(m_hwndConnStatus, Status_Connected);
	}
}

void CConfig::OnLaunchWinLIRCBtn()
{
	if (IsWinLIRCRunning())
		return;

	if (LaunchWinLIRC() == -1) { // Path not found
		CConfigOptions dlg;
		dlg.BrowseForWinLIRC();
		LaunchWinLIRC();
	}

	Sleep(200);
	if (IsWinLIRCRunning()) {
		EnableWindow(m_hwndLaunchWinLIRCBtn, FALSE);
		SetWindowText(m_hwndWinLIRCStatus, Status_Running);
	}
}

void CConfig::OnHelpBtn()
{
	CHelp dlg;
	dlg.DoModal();
}

void CConfig::OnListDblClk()
{
	OnModifyBtn();
}

//----------------------------------------------------------------------------
// The Callback
//----------------------------------------------------------------------------

BOOL CALLBACK CConfig::DialogCallback(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		// In response to a WM_INITDIALOG message, the dialog box procedure should return zero if it calls the SetFocus function to set the focus to one of the controls in the dialog box. Otherwise, it should return nonzero, in which case the system sets the focus to the first control in the dialog box that can be given the focus.
		DialogInit(hwndDlg);

		return TRUE;
		break;
	}
	case WM_PN_DIALOGSAVE :
	{
		// save work

		// QCD takes care of destroying the dialog
		return TRUE;
	}
	case WM_COMMAND :
	{
		switch (wParam)
		{
		case IDC_RECONNECT_BTN :
			OnReconnectBtn();
			break;
		case IDC_LAUNCH_WINLIRC_BTN :
			OnLaunchWinLIRCBtn();
			break;			
		case IDC_ADD_BTN :
			OnAddBtn();
			break;
		case IDC_REMOVE_BTN :
			OnRemoveBtn();
			break;
		case IDC_MODIFY_BTN :
			OnModifyBtn();
			break;
		case IDC_OPTIONS_BTN :
			OnOptionsBtn();
			break;
		case IDC_HELP_BTN :
			OnHelpBtn();
			break;
		} // Switch

	}
	case WM_NOTIFY :
	{
		switch(LOWORD(wParam)) // hit control
		{
		case IDC_LIST :
			if(((LPNMHDR)lParam)->code == NM_DBLCLK)
				OnListDblClk();
			break;

		} // switch		

		break;
	}
	} // switch
	return FALSE;
}

//----------------------------------------------------------------------------
// Get/Set functions
//----------------------------------------------------------------------------

INT CConfig::GetPageID()
{
	return m_iPageID;
}

VOID CConfig::SetSettings(Settings* pSettings)
{
	m_pSettings = pSettings;
}