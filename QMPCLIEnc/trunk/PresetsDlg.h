#pragma once

#include "stdafx.h"

#include "QMPCLIEnc.h"
#include "TreeCtrl.h"
#include "ToolTipDialog.h"
#import <msxml3.dll>


//////////////////////////////////////////////////////////////////////////

class CPresetsDlg :
	public CDialogImpl< CPresetsDlg >, 
	public CWinDataExchange< CPresetsDlg >, 
	public CToolTipDialog< CPresetsDlg >
{
public:
	CPresetsDlg(void)
		: m_strPath(_T(""))
		, m_strParameter(_T(""))
		, m_strExtension(_T(""))
	{
		CoInitialize( NULL);
	}
	~CPresetsDlg(void)
	{
		m_pXMLDom->Release(); // release reference count
		CoUninitialize();
	}


public:
	enum { IDD = IDD_PRESETS };

	// Maps
	BEGIN_MSG_MAP(CPresetsDlg)
		CHAIN_MSG_MAP(CToolTipDialog< CPresetsDlg >)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_NEW, OnNew)
		COMMAND_ID_HANDLER_EX(IDC_DELETE, OnDelete)
		COMMAND_ID_HANDLER_EX(IDC_SAVE, OnSave)
		COMMAND_ID_HANDLER_EX(IDC_LOAD, OnLoad)
		COMMAND_ID_HANDLER_EX(IDC_BP, OnBrowsePath)
		COMMAND_ID_HANDLER_EX(IDC_UPDATE, OnUpdate)
		MESSAGE_HANDLER_EX(WM_PN_DIALOGSAVE, OnDialogSave)
		NOTIFY_HANDLER_EX(IDC_EP_TREE, TVN_SELCHANGING, OnTvnSelChanging)
		NOTIFY_HANDLER_EX(IDC_EP_TREE, TVN_SELCHANGED, OnTvnSelChanged)
		NOTIFY_HANDLER_EX(IDC_EP_TREE, TVN_ENDLABELEDIT, OnTvnEndLabelEdit)
		NOTIFY_HANDLER_EX(IDC_EP_TREE, NM_DBLCLK, OnNMDBlck)
		COMMAND_CODE_HANDLER_EX(EN_CHANGE, OnENChange)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	// DDX
	BEGIN_DDX_MAP(CPresetsDlg)
		DDX_CONTROL(IDC_EP_TREE, m_ctrlEPTree)
		DDX_CONTROL_HANDLE(IDC_SAVE, m_ctrlSave)
		DDX_CONTROL_HANDLE(IDC_UPDATE, m_ctrlUpdate)
		DDX_TEXT(IDC_PAT, m_strPath)
		DDX_TEXT(IDC_PAR, m_strParameter)
		DDX_TEXT(IDC_EXT, m_strExtension)
	END_DDX_MAP()

	// Message handlers
	BOOL OnInitDialog(HWND hwndFocus, LPARAM lParam)
	{
		// DDX controls, Hook it.
		DoDataExchange( FALSE);

		// save the settings dialog box
		m_hwndSettings = (HWND)lParam;

		// disable update/save button default
		m_ctrlUpdate.EnableWindow( FALSE);
		m_ctrlSave.EnableWindow( FALSE);

		// initialize the encoder preset tree
		HRESULT hr = m_pXMLDom.CreateInstance(__uuidof(MSXML2::DOMDocument30));
		if ( FAILED(hr)) {
			MessageBox( _T("Failed to instantiate DOMDocument30 class"), _T("ERROR"), MB_OK | MB_ICONSTOP);
			return FALSE;
		}

		// Add reference count
		m_pXMLDom->AddRef();

		// load preset file
		m_pXMLDom->async = VARIANT_FALSE;
		if ( m_pXMLDom->load( g_szEPFile) != VARIANT_TRUE) {
			CString str = _T("Failed load xml data from file: ");
			str += g_szEPFile;
			MessageBox( str, _T("ERROR"), MB_OK | MB_ICONSTOP);

			m_pXMLDom->loadXML( "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Encoders></Encoders>");
		}

		// fill tree view
		_fillTree();

		// set property of tool tip control
		GetTT().SetDelayTime( TTDT_AUTOPOP, 10000); // set the show delay to 10 seconds
		GetTT().SetMaxTipWidth( 500); // enable multi-line tooltips

		// set icon and title of tool tip control
		TTSetTitle( IDC_EP_TREE, 1, "Tips");
		TTSetTitle( IDC_PAR, 1, "REQUIRED");

		return TRUE;
	}

	void OnNew(UINT uCode, int nID, HWND hwndCtrl)
	{
		CTreeItem ti;
		CString strNew;
		MSXML2::IXMLDOMElementPtr peNew, peParent;
		MSXML2::IXMLDOMNodeListPtr pnl;
		CTreeItem tiNew, tiParent;

		ti = m_ctrlEPTree.GetSelectedItem();
		if ( !ti.IsNull()) {
			UINT index;

			// Prepare insert position and data for database and tree view
			if ( ti.HasChildren()) { // for root item which has children -- create a new root item
				// generate a unique encoder name
				strNew = _T("New Encoder");
				pnl = m_pXMLDom->selectNodes( _getXPathFromItem( ti, strNew));
				index = 1;
				while ( pnl->length > 0) { // not unique, calculate a new name
					strNew.Format( _T("New Encoder (%d)"), index++);
					pnl = m_pXMLDom->selectNodes( _getXPathFromItem( ti, strNew));
				}

				peNew = m_pXMLDom->createElement( "Encoder");
				peNew->setAttribute( "name", _variant_t(strNew));

				peParent = m_pXMLDom->selectSingleNode( _getXPathFromItem( ti))->parentNode;

				tiParent = ti.GetParent();
			} else if ( !ti.GetParent().IsNull()) { // for child item -- create a new child item
				// generate a unique preset name
				strNew = _T("New Preset");
				pnl = m_pXMLDom->selectNodes( _getXPathFromItem( ti, strNew));
				index = 1;
				while ( pnl->length > 0) { // not unique, calculate a new name
					strNew.Format( _T("New Preset (%d)"), index++);
					pnl = m_pXMLDom->selectNodes( _getXPathFromItem( ti, strNew));
				}

				peNew = m_pXMLDom->createElement( "Preset");
				peNew->setAttribute( "name", _variant_t(strNew));
				peNew->setAttribute( "path", "");
				peNew->setAttribute( "parameter", "");
				peNew->setAttribute( "extension", "");

				peParent = m_pXMLDom->selectSingleNode( _getXPathFromItem( ti))->parentNode;

				tiParent = ti.GetParent();
			} else if ( ti.GetParent().IsNull() && !ti.HasChildren()) { // for empty root item -- create a new child item
				// generate a unique preset name
				strNew = _T("New Preset");

				peNew = m_pXMLDom->createElement( "Preset");
				peNew->setAttribute( "name", _variant_t(strNew));
				peNew->setAttribute( "path", "");
				peNew->setAttribute( "parameter", "");
				peNew->setAttribute( "extension", "");

				peParent = m_pXMLDom->selectSingleNode( _getXPathFromItem( ti));

				tiParent = ti;
			}
		} else { // for empty tree view
			// generate a unique encoder name
			strNew = _T("New Encoder");

			peNew = m_pXMLDom->createElement( "Encoder");
			peNew->setAttribute( "name", _variant_t(strNew));

			peParent = m_pXMLDom->documentElement;

			tiParent = ti.GetParent();
		}

		// append the new child into database
		peParent->appendChild( (MSXML2::IXMLDOMNodePtr)peNew);

		// append a child node to tree view
		tiNew = tiParent.AddTail( strNew, -1);
		tiNew.EnsureVisible();
		tiNew.Select();
		tiNew.EditLabel();

		// enable save button
		m_ctrlSave.EnableWindow( TRUE);
	}

	void OnDelete(UINT uCode, int nID, HWND hwndCtrl)
	{
		CTreeItem ti = m_ctrlEPTree.GetSelectedItem();

		MSXML2::IXMLDOMNodePtr pn = m_pXMLDom->selectSingleNode( _getXPathFromItem( ti));

		if ( ti.HasChildren()) {
			CString buf, tmp;

			buf = _T("Are you sure to delete encoder: \"");
			ti.GetText( tmp);
			buf += tmp + _T("\"?\nNote: All of its content will be deleted!");
			if ( MessageBox( buf, _T("Warning!"), MB_YESNO|MB_ICONWARNING) != IDYES)
				return ;
		}

		// remove it from database
        pn->parentNode->removeChild( pn);

		// update UI
		ti.Delete();
		m_ctrlSave.EnableWindow( TRUE);
	}

	void OnLoad(UINT uCode, int nID, HWND hwndCtrl)
	{
		// "Open file..." file dialog
		CFileDialog fileDlg( TRUE, _T("xml"), NULL, OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST, _T("Encoder Preset (*.xml)\0*.xml\0All (*.*)\0*.*\0"), m_hWnd);

		if ( fileDlg.DoModal() == IDOK) {
			if ( m_pXMLDom->load( fileDlg.m_szFileName) == VARIANT_TRUE) {
				// fill tree view
				_fillTree();

				lstrcpy( g_szEPFile, fileDlg.m_szFileName);

				// redraw dialog to avoid tree-view non-repainting
				InvalidateRect( NULL, TRUE);
			} else {
				CString str = _T("Failed load xml data from file: ");
				str += g_szEPFile;
				MessageBox( str, _T("ERROR"), MB_OK | MB_ICONSTOP);

				m_pXMLDom->loadXML( "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Encoders></Encoders>");

				m_ctrlEPTree.DeleteAllItems();
			}
		}
	}

	void OnUpdate(UINT uCode, int nID, HWND hwndCtrl)
	{
		_doUpdate();

		m_ctrlUpdate.EnableWindow( FALSE); // disable self
		m_ctrlSave.EnableWindow( TRUE); // enable saving
	}

	void OnSave(UINT uCode, int nID, HWND hwndCtrl)
	{
		m_pXMLDom->save( g_szEPFile);
		m_ctrlSave.EnableWindow( FALSE);
	}

	void OnBrowsePath(UINT uCode, int nID, HWND hwndCtrl)
	{
		CFileDialog fileDlg( TRUE, _T("exe"), NULL, OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST, _T("Executables (*.exe;*.vbs)\0*.exe;*.vbs\0All (*.*)\0*.*\0"), m_hWnd);

		if ( fileDlg.DoModal() == IDOK) {
			m_strPath = fileDlg.m_szFileName;

			CEdit edit = GetDlgItem( IDC_PAT);
			edit.SetFocus();

			DoDataExchange( FALSE);

			edit.SetSelAll();
		}
	}

	LRESULT OnTvnSelChanging(LPNMHDR pNMHDR)
	{
		if ( m_ctrlUpdate.IsWindowEnabled()) {
			if ( MessageBox( _T("A preset has been modified!\nUpdate it now?"), _T("QMPCLIEnc"), MB_YESNO|MB_ICONINFORMATION) == IDYES)
				_doUpdate();
		}

		return FALSE; // enable selection from changing
	}

	LRESULT OnTvnSelChanged(LPNMHDR pNMHDR)
	{
		CTreeItem ti = m_ctrlEPTree.GetSelectedItem();
		BOOL isRoot = ti.GetParent().IsNull();

		if ( !isRoot) {
			MSXML2::IXMLDOMElementPtr pe = m_pXMLDom->selectSingleNode( _getXPathFromItem( ti));
			m_strPath = (LPCTSTR)_bstr_t(pe->getAttribute( "path"));
			m_strParameter = (LPCTSTR)_bstr_t(pe->getAttribute( "parameter"));
			m_strExtension = (LPCTSTR)_bstr_t(pe->getAttribute( "extension"));
		} else {
			m_strPath = _T("");
			m_strParameter = _T("");
			m_strExtension = _T("");
		}

		GetDlgItem( IDC_PAT).EnableWindow( !isRoot);
		GetDlgItem( IDC_BP).EnableWindow( !isRoot);
		GetDlgItem( IDC_PAR).EnableWindow( !isRoot);
		GetDlgItem( IDC_EXT).EnableWindow( !isRoot);

		m_ctrlUpdate.EnableWindow( FALSE);

		DoDataExchange( FALSE);

		return TRUE; // ignored
	}

	LRESULT OnTvnEndLabelEdit(LPNMHDR pNMHDR)
	{
		LPNMTVDISPINFO lptvdi = (LPNMTVDISPINFO)pNMHDR;

		if ( lptvdi->item.pszText != NULL) { // yes, not cancel edit, so save it when modified
			CTreeItem ti = m_ctrlEPTree.GetSelectedItem();

			MSXML2::IXMLDOMElementPtr pe = m_pXMLDom->selectSingleNode( _getXPathFromItem( ti));
			if ( pe->getAttribute( "name") != _variant_t(lptvdi->item.pszText)) {
				// verify unique
				MSXML2::IXMLDOMNodeListPtr pnl = m_pXMLDom->selectNodes( _getXPathFromItem( ti, lptvdi->item.pszText));
				if ( !pnl->length) {
					pe->setAttribute( "name", lptvdi->item.pszText);
					m_ctrlSave.EnableWindow( TRUE);

					return TRUE; // update the item's label
				}
			}
		}

		return FALSE; // avoid updating a item's label
	}

	LRESULT OnNMDBlck(LPNMHDR pNMHDR)
	{
		CTreeItem ti = m_ctrlEPTree.GetSelectedItem();
		if ( ti.GetParent().HasChildren()) { // only for preset
			MSXML2::IXMLDOMElementPtr pe;
			CString path, parameter, extension;

			// select the item from XML database
			pe = m_pXMLDom->selectSingleNode( _getXPathFromItem( ti));

			// get value of selected child item.
			path = (LPCSTR)_bstr_t(pe->getAttribute( "path"));
			parameter = (LPCSTR)_bstr_t(pe->getAttribute( "parameter"));
			extension = (LPCSTR)_bstr_t(pe->getAttribute( "extension"));

			// filling setting page's fields
			::SetDlgItemText( m_hwndSettings, IDC_PATH, path);
			::SetDlgItemText( m_hwndSettings, IDC_PARAMETER, parameter);
			::SetDlgItemText( m_hwndSettings, IDC_EXTENSION, extension);

			// switch to settings page
			CTabCtrl tab;
			tab.Attach( GetParent().GetDlgItem( IDC_TAB));
			tab.SetCurFocus( 0);
			tab.Detach();
		}

		return FALSE; // enable process double click to expand a root item
	}

	void OnENChange(UINT uCode, int nID, HWND hwndCtrl)
	{
		CTreeItem ti = m_ctrlEPTree.GetSelectedItem();

		if ( !ti.IsNull() && ti.GetParent().HasChildren() && GetFocus() == hwndCtrl) {
			DoDataExchange( TRUE);

			MSXML2::IXMLDOMElementPtr pe = m_pXMLDom->selectSingleNode( _getXPathFromItem( ti));
			if ( 
				pe->getAttribute( "path") != _variant_t( m_strPath) || 
				pe->getAttribute( "parameter") != _variant_t( m_strParameter) || 
				pe->getAttribute( "extension") != _variant_t( m_strExtension)
				) m_ctrlUpdate.EnableWindow( TRUE);
		}
	}

	LRESULT OnDialogSave(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		BOOL update = m_ctrlUpdate.IsWindowEnabled();
		BOOL save = m_ctrlSave.IsWindowEnabled();
		if ( update || save) {
			if ( MessageBox( _T("Some Presets have been modified!\nSave them now?"), _T("QMPCLIEnc"), MB_YESNO|MB_ICONINFORMATION) == IDYES) {
				if ( update) _doUpdate();

				m_pXMLDom->save( g_szEPFile);
			}
		}

		return TRUE;
	}


private: // Internal function
	_bstr_t _getXPathFromItem(CTreeItem & ti, CString name = _T(""))
	{
		CString xpath, tmp;

		xpath = "/Encoders/Encoder[@name=\"";

		if ( !ti.GetParent().IsNull()) { // It is a child item
			ti.GetParent().GetText( tmp);
			xpath += tmp + "\"]/Preset[@name=\"";
		}

		ti.GetText( tmp);
		xpath += (name.IsEmpty() ? tmp : name) + "\"]";

		return _bstr_t(xpath);
	}

	void _doUpdate(void)
	{
		DoDataExchange( TRUE);

		CTreeItem ti = m_ctrlEPTree.GetSelectedItem();

		if ( ti.GetParent().HasChildren()) { // only for preset
			MSXML2::IXMLDOMElementPtr pe = m_pXMLDom->selectSingleNode( _getXPathFromItem( ti));
			pe->setAttribute( "path", _bstr_t(m_strPath));
			pe->setAttribute( "parameter", _bstr_t(m_strParameter));
			pe->setAttribute( "extension", _bstr_t(m_strExtension));
		}
	}

	void _fillTree(void)
	{
		// reset all items first
		m_ctrlEPTree.DeleteAllItems();

		// create tree
		MSXML2::IXMLDOMNodeListPtr pnl = m_pXMLDom->selectNodes( "/Encoders/Encoder[@name]");
		MSXML2::IXMLDOMNodeListPtr pnl_p;
		for ( int i = 0; i < pnl->length; ++i) {
			// get encoder name
			for ( int j = 0; j < pnl->item[i]->attributes->length; ++j) {
				if ( !lstrcmp( pnl->item[i]->attributes->item[j]->nodeName, "name")) {
					// insert encoder item
					CTreeItem parent = m_ctrlEPTree.InsertItem( _bstr_t(pnl->item[i]->attributes->item[j]->nodeValue), TVI_ROOT, TVI_LAST);

					// insert preset items
					CString str = _T("/Encoders/Encoder[@name=\"");
					str += _bstr_t(pnl->item[i]->attributes->item[j]->nodeValue);
					str += "\"]/Preset";
					pnl_p = m_pXMLDom->selectNodes( _bstr_t( str));
					for ( j = 0; j < pnl_p->length; ++j) {
						for ( int k = 0; k < pnl_p->item[j]->attributes->length; ++k) {
							if ( !lstrcmp( pnl_p->item[j]->attributes->item[k]->nodeName, "name")) {
								parent.AddTail( _bstr_t(pnl_p->item[j]->attributes->item[k]->nodeValue), -1);

								break; // finish processing preset name
							}
						}
					}

					break; // finish processing encoder name
				}
			}
		}

		m_ctrlEPTree.SelectItem( m_ctrlEPTree.GetFirstVisibleItem());
	}


private: // DDX Vars
	CTreeCtrl m_ctrlEPTree;
	CButton m_ctrlSave;
	CString m_strPath;
	CString m_strParameter;
	CString m_strExtension;
	CButton m_ctrlUpdate;

private: // XML Vars
	MSXML2::IXMLDOMDocument2Ptr m_pXMLDom;

private: // Internal Vars
	HWND m_hwndSettings;
};

