#pragma once

#include "stdafx.h"

#include "QMPdAPDecMgr.h"


//////////////////////////////////////////////////////////////////////////

class CConfigDlg :
	public CDialogImpl<CConfigDlg>, 
	public CWinDataExchange<CConfigDlg>
{
public:
	CConfigDlg(void) {}
	~CConfigDlg(void) {}


public:
	enum { IDD = IDD_CONFIG };

	// Maps
	BEGIN_MSG_MAP(CConfigDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER_EX(IDC_PLUGINS_DIR, OnPluginsDir)
		COMMAND_ID_HANDLER_EX(IDC_CLOSE, OnClose)
		COMMAND_ID_HANDLER_EX(IDC_INFORMATION, OnInformation)
		COMMAND_ID_HANDLER_EX(IDC_MOVEUP, OnMoveItem)
		COMMAND_ID_HANDLER_EX(IDC_MOVEDOWN, OnMoveItem)
		NOTIFY_HANDLER_EX(IDC_PLUGINS_LISTVIEW, LVN_ITEMCHANGED, OnLvnItemchanged)
		NOTIFY_HANDLER_EX(IDC_PLUGINS_LISTVIEW, NM_DBLCLK, OnNMDBClickPluginsListView)
		NOTIFY_HANDLER_EX(IDC_PLUGINS_LISTVIEW, NM_RCLICK, OnNMRClickPluginsListView)
	END_MSG_MAP()

	// DDX
	BEGIN_DDX_MAP(CConfigDlg)
		DDX_TEXT(IDC_PLUGINS_DIR, g_strPluginDir)
		DDX_CONTROL(IDC_PLUGINS_LISTVIEW, m_ctlPlugins)
		DDX_CONTROL(IDC_URL_PLUGINS, m_ctlURLPlugins)
		DDX_CONTROL_HANDLE(IDC_INFORMATION, m_ctlInformation)
		DDX_CONTROL_HANDLE(IDC_MOVEUP, m_ctlMoveUp)
		DDX_CONTROL_HANDLE(IDC_MOVEDOWN, m_ctlMoveDown)
		DDX_CONTROL_HANDLE(IDC_WARNING, m_ctlWarning)
	END_DDX_MAP()

	// Message handlers
	BOOL CConfigDlg::OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		// Save the pointer to myself for self-destroy
		m_pME = (CConfigDlg **)lParam;

		// DDX controls, Hook it.
		if ( !g_strPluginDir.IsEmpty())
			DoDataExchange( FALSE, IDC_PLUGINS_DIR);
		DoDataExchange( FALSE, IDC_REFRESH);
		DoDataExchange( FALSE, IDC_PLUGINS_LISTVIEW);
		DoDataExchange( FALSE, IDC_URL_PLUGINS);
		DoDataExchange( FALSE, IDC_INFORMATION);
		DoDataExchange( FALSE, IDC_MOVEUP);
		DoDataExchange( FALSE, IDC_MOVEDOWN);
		DoDataExchange( FALSE, IDC_WARNING);

		// Update the UI
		m_ctlURLPlugins.SetHyperLinkExtendedStyle( HLINK_UNDERLINEHOVER);
		m_ctlURLPlugins.SetHyperLink( "http://www.dbpoweramp.com/codec-central.htm");

		m_ctlPlugins.AddColumn( "Description", 0);
		m_ctlPlugins.AddColumn( "Extensions", 1);

		for ( ITMod it = g_Modules.begin(); it != g_Modules.end(); ++it) {
			int nItem = it-g_Modules.begin();
			m_ctlPlugins.AddItem( nItem, 0, ((*it).strDesc+"    ["+PathFindFileName( (*it).strFilePath)+"]"));
			m_ctlPlugins.AddItem( nItem, 1, (*it).strExts);
			m_ctlPlugins.SetCheckState( nItem, (*it).bEnabled);
		}

		if ( g_Modules.empty()) {
			CRect rect;
			m_ctlPlugins.GetClientRect( &rect);
			m_ctlPlugins.SetColumnWidth( 0, (int)(rect.Width()*80.0/100));
		} else {
			m_ctlPlugins.SetColumnWidth( 0, LVSCW_AUTOSIZE);
			m_ctlPlugins.SetColumnWidth( 1, LVSCW_AUTOSIZE);
		}

		m_ctlInformation.EnableWindow( FALSE);
		m_ctlMoveUp.EnableWindow( FALSE);
		m_ctlMoveDown.EnableWindow( FALSE);
		m_ctlWarning.ShowWindow( SW_HIDE);

		return TRUE;
	}
	void CConfigDlg::OnPluginsDir(UINT uCode, int nID, HWND hwndCtrl)
	{
		CFolderDialog fldrDlg( m_hWnd, _T("Select a folder to load dBpowerAMP's decoder plug-ins: "), BIF_NEWDIALOGSTYLE);

		fldrDlg.SetInitialFolder( g_strPluginDir);
		if ( fldrDlg.DoModal() == IDOK) {
			CString path = fldrDlg.GetFolderPath();
			if ( path.CompareNoCase( g_strPluginDir)) {
				MessageBox( "You need: \n* restart this manager plug-in \nor \n*restart player \nto take changed effert!", "NOTE", MB_OK|MB_ICONINFORMATION);
				g_strPluginDir = path;

				m_ctlWarning.ShowWindow( SW_SHOW);

				DoDataExchange( FALSE);
			}
		}
	}
	void OnInformation(UINT uCode, int nID, HWND hwndCtrl)
	{
		int index = m_ctlPlugins.GetSelectedIndex();
		if ( index >= 0) {
			g_Modules[index].ShowAboutOptionsPage();
		}
	}
	void OnMoveItem(UINT uCode, int nID, HWND hwndCtrl)
	{
		// UI and vector must have the same order exactly!!
		int index = m_ctlPlugins.GetSelectedIndex();
		if ( index >= 0) {
			CString desc, exts;
			int pos; // old position
			BOOL state;

			pos = index;

			// save text and state
			state = m_ctlPlugins.GetCheckState( index);
			m_ctlPlugins.GetItemText( index, 0, desc);
			m_ctlPlugins.GetItemText( index, 1, exts);

			// delete it from the old position
			m_ctlPlugins.DeleteItem( index);

			// update to the new position
			if ( nID == IDC_MOVEUP)
				--index;
			else
				++index;

			// swap the item in plug-in vector
			g_Modules[pos].uOrder = index;
			g_Modules[index].uOrder = pos;
			_sort_plugins();

			// synchronize the playing plug-in
			if ( decoderInfo.pDecoder && decoderInfo.DecMod.uOrder == pos)
				decoderInfo.DecMod.uOrder = index;

			// insert into a position
			m_ctlPlugins.AddItem( index, 0, desc);
			m_ctlPlugins.AddItem( index, 1, exts);
			// restore text and state
			m_ctlPlugins.SetCheckState( index, state);
			m_ctlPlugins.SelectItem( index);
			// other UI issue
			m_ctlPlugins.SetSelectionMark( index);
			m_ctlPlugins.EnsureVisible( index, FALSE);
			::SetFocus( hwndCtrl); // focus on our up/down button
			m_ctlMoveUp.EnableWindow( index > 0);
			m_ctlMoveDown.EnableWindow( index < m_ctlPlugins.GetItemCount()-1);
		}
	}
	void OnClose(UINT uCode = 0, int nID = 0, HWND hwndCtrl = NULL)
	{
		DestroyWindow();
	}
	void OnFinalMessage(HWND hwnd)
	{
		if ( m_pME) { // destroy myself automatically^_^
			*m_pME = NULL;
			delete this;
		}
	}
	LRESULT OnLvnItemchanged(LPNMHDR pNMHDR)
	{
		SetMsgHandled( FALSE);

		LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

		BOOL bPrevState = (BOOL)(((lpnmlv->uOldState & LVIS_STATEIMAGEMASK)>>12)-1); // checked before?
		BOOL bChecked = (BOOL)(((lpnmlv->uNewState & LVIS_STATEIMAGEMASK)>>12)-1);   // checked current?
		if ( bPrevState != bChecked && bPrevState >= 0) { // item should change its loading state
			if ( bChecked) { // unchecked before, so process check event
				bool ok = _load_one_dll( g_Modules[lpnmlv->iItem], lpnmlv->iItem);

				// Update item
				m_ctlPlugins.SetItemText( lpnmlv->iItem, 0, (g_Modules[lpnmlv->iItem].strDesc+"    ["+PathFindFileName( g_Modules[lpnmlv->iItem].strFilePath)+"]"));
				CString str = g_Modules[lpnmlv->iItem].strExts;
				m_ctlPlugins.SetItemText( lpnmlv->iItem, 1, g_Modules[lpnmlv->iItem].strExts);
				if ( !ok) m_ctlPlugins.SetCheckState( lpnmlv->iItem, FALSE); // uncheck it when false

				// the "Information" button
				if ( lpnmlv->iItem == m_ctlPlugins.GetSelectedIndex())
					m_ctlInformation.EnableWindow( ok);
			} else { // Now, the uncheck event
				// stop if playing
				if ( decoderInfo.pDecoder && decoderInfo.DecMod.uOrder == (lpnmlv->iItem))
					Stop( decoderInfo.playingFile, STOPFLAG_FORCESTOP);

				// unload plug-in
				_free_one_dll( g_Modules[lpnmlv->iItem]);

				// the "Information" button
				if ( lpnmlv->iItem == m_ctlPlugins.GetSelectedIndex())
					m_ctlInformation.EnableWindow( FALSE);
			}
		} else { // item selection(highlight)
			int index = m_ctlPlugins.GetSelectedIndex();

			// load plug-in when selected and checked
			if ( index >= 0 && m_ctlPlugins.GetCheckState( index) && g_Modules[index].hMod == NULL) {
				if ( !_load_one_dll( g_Modules[index], index))
					m_ctlPlugins.SetCheckState( index, FALSE);
			}

			m_ctlInformation.EnableWindow( index>=0 && m_ctlPlugins.GetCheckState( index));
			m_ctlMoveUp.EnableWindow( index > 0);
			m_ctlMoveDown.EnableWindow( index >=0 && index != m_ctlPlugins.GetItemCount()-1);
		}

		return TRUE;
	}
	LRESULT OnNMDBClickPluginsListView(LPNMHDR pNMHDR)
	{
		int index = m_ctlPlugins.GetSelectedIndex();
		if ( index >= 0 && m_ctlPlugins.GetCheckState( index)) {
			g_Modules[index].ShowAboutOptionsPage();
			return TRUE;
		} else {
			return FALSE;
		}
	}
	LRESULT OnNMRClickPluginsListView(LPNMHDR pNMHDR)
	{
		int index = m_ctlPlugins.GetSelectedIndex();
		if ( index >= 0) {
			LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;
			CMenu menuDelete;
			menuDelete.CreatePopupMenu();
			menuDelete.InsertMenu( 0, MF_BYPOSITION, 1, "Delete plug-in");
			POINT pt;
			::GetCursorPos( &pt);
			if ( 1 == menuDelete.TrackPopupMenu( TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_TOPALIGN|TPM_HORIZONTAL|TPM_NONOTIFY|TPM_RETURNCMD, pt.x, pt.y, this->m_hWnd)) {
				SHFILEOPSTRUCT fops;
				TCHAR path[MAX_PATH];

				// stop if playing
				if ( decoderInfo.pDecoder && decoderInfo.DecMod.uOrder == index)
					Stop( decoderInfo.playingFile, STOPFLAG_FORCESTOP);

				// unload plug-in
				_free_one_dll( g_Modules[index]);

				ZeroMemory( path, MAX_PATH);
				lstrcpy( path, g_Modules[index].strFilePath);

				ZeroMemory( &fops, sizeof(SHFILEOPSTRUCT));
				fops.hwnd = this->m_hWnd;
				fops.wFunc = FO_DELETE;
				fops.fFlags = FOF_ALLOWUNDO;
				fops.pFrom = path;

				if ( !SHFileOperation( &fops)) { // successful
					if ( fops.fAnyOperationsAborted)
						m_ctlPlugins.SetCheckState( index, FALSE);
					else {
						m_ctlPlugins.DeleteItem( index);
						g_Modules.erase( g_Modules.begin()+index);
						_sort_plugins();
					}
				} else {
					m_ctlPlugins.SetCheckState( index, FALSE);
				}
			}
		}

		return TRUE;
	}


private: // DDX Vars
	CConfigDlg **m_pME;
	CCheckListViewCtrl m_ctlPlugins;
	CHyperLink m_ctlURLPlugins;
	CButton m_ctlInformation;
	CButton m_ctlMoveUp, m_ctlMoveDown;
	WTL::CStatic m_ctlWarning;
};

