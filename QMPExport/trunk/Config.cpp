// Config.cpp
//
//////////////////////////////////////////////////////////////////////

// #include <stdio.h>

#include "QMPGeneralDLL.h"
#include "Config.h"

//////////////////////////////////////////////////////////////////////

static const _TCHAR  INI_SECTION[]   = _T("QMPExport");
static const char    INI_SECTION_A[] = "QMPExport";

//////////////////////////////////////////////////////////////////////

void LoadSettings()
{
	int nRet = 0;
	WCHAR szBuffer[MAX_PATH] = {0};
	QString strIni;

	QMPCallbacks.Service(opGetPluginSettingsFile, szBuffer, sizeof(szBuffer), 0);
	strIni.SetUnicode(szBuffer);

	//Settings.bFirstRun = GetPrivateProfileInt(INI_SECTION, _T("FirstRun"), 1, strIni);
	
	GetPrivateProfileString(INI_SECTION, _T("NowPlayingPath"), NULL, szBuffer, NUMOFTCHARS(szBuffer), strIni);
	Settings.strNowPlayingPath.SetUnicode(szBuffer);

	GetPrivateProfileString(INI_SECTION, _T("XmlExportPath"), NULL, szBuffer, NUMOFTCHARS(szBuffer), strIni);
	Settings.strXmlExportPath.SetUnicode(szBuffer);

	GetPrivateProfileString(INI_SECTION, _T("HtmlExportPath"), NULL, szBuffer, NUMOFTCHARS(szBuffer), strIni);
	Settings.strHtmlExportPath.SetUnicode(szBuffer);


	// Defaullt initization
	if (Settings.strNowPlayingPath.Length() == 0 || Settings.strXmlExportPath.Length() == 0 || Settings.strHtmlExportPath.Length() == 0)
	{
		// Get default path
		WCHAR strPath[MAX_PATH] = {0};
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		if (osvi.dwMajorVersion >= 6) { // Vista+
			QMPCallbacks.Service(opGetSettingsFolder, strPath, sizeof(strPath), 0);
		}
		else { // XP or below
			QMPCallbacks.Service(opGetPlayerFolder, strPath, sizeof(strPath), 0);
		}

		Settings.strNowPlayingPath.SetUnicode(strPath);
		Settings.strNowPlayingPath.AppendUnicode(L"\\nowplaying.xml");

		Settings.strXmlExportPath.SetUnicode(strPath);
		Settings.strXmlExportPath.AppendUnicode(L"\\Playlist.xml");

		Settings.strHtmlExportPath.SetUnicode(strPath);
		Settings.strHtmlExportPath.AppendUnicode(L"\\Playlist.xhtml");
	}
}

//////////////////////////////////////////////////////////////////////

void SaveSettings()
{
	WCHAR szBuffer[MAX_PATH] = {0};
	QString strIni;

	QMPCallbacks.Service(opGetPluginSettingsFile, szBuffer, sizeof(szBuffer), 0);
	strIni.SetUnicode(szBuffer);

	//_stprintf_s(szBuffer, NUMOFTCHARS(szBuffer), _T("%d"), Settings.bFirstRun);
	//WritePrivateProfileString(INI_SECTION, _T("FirstRun"), szBuffer, strIni);

	WritePrivateProfileString(INI_SECTION, _T("NowPlayingPath"), Settings.strNowPlayingPath, strIni);
	WritePrivateProfileString(INI_SECTION, _T("XmlExportPath"),  Settings.strXmlExportPath, strIni);
	WritePrivateProfileString(INI_SECTION, _T("HtmlExportPath"), Settings.strHtmlExportPath, strIni);
}

//////////////////////////////////////////////////////////////////////

void CreateConfigDlg(HINSTANCE hInstance, HWND hAppHwnd)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), hAppHwnd, ConfigDlgProc);
}

//////////////////////////////////////////////////////////////////////

BOOL CALLBACK ConfigDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL g_bPasswordChanged = FALSE;
	BOOL bReturn = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		// CheckDlgButton(hwndDlg, IDC_CONFIG_PROXY, Settings.bUseProxy);
		SetDlgItemText(hwndDlg, IDC_CONFIG_NOWPLAYINGPATH, Settings.strNowPlayingPath);
		SetDlgItemText(hwndDlg, IDC_CONFIG_XMLEXPORTPATH,  Settings.strXmlExportPath);
		SetDlgItemText(hwndDlg, IDC_CONFIG_HTMLEXPORTPATH, Settings.strHtmlExportPath);

		// Get preferences placement and place accordingly
		HWND hWndPref = (HWND)QMPCallbacks.Service(opGetPropertiesWnd, NULL, 0, 0);
		if (hWndPref) {
			WINDOWPLACEMENT DlgPos, PrefPos;
			DlgPos.length  = sizeof(WINDOWPLACEMENT);
			PrefPos.length = sizeof(WINDOWPLACEMENT);

			GetWindowPlacement(hwndDlg, &DlgPos);
			GetWindowPlacement(hWndPref, &PrefPos);
			int nWidth  = DlgPos.rcNormalPosition.right - DlgPos.rcNormalPosition.left;
			int nHeight = DlgPos.rcNormalPosition.bottom - DlgPos.rcNormalPosition.top;
			DlgPos.rcNormalPosition.left   = PrefPos.rcNormalPosition.left + 220;
			DlgPos.rcNormalPosition.top    = PrefPos.rcNormalPosition.top + 60;
			DlgPos.rcNormalPosition.right  = DlgPos.rcNormalPosition.left + nWidth;
			DlgPos.rcNormalPosition.bottom = DlgPos.rcNormalPosition.top + nHeight;
			
			SetWindowPlacement(hwndDlg, &DlgPos);
		}

		bReturn = TRUE;
		break;
	}
	case WM_CLOSE :
	{
		EndDialog(hwndDlg, TRUE);
		bReturn = TRUE;
		break;
	}
	case WM_COMMAND :
	{
		switch (wParam)
		{
		case IDOK :
		{
			_TCHAR szBuffer[128];

			// Validate and save
			// Settings.bUseProxy = IsDlgButtonChecked(hwndDlg, IDC_CONFIG_PROXY);

			GetDlgItemText(hwndDlg, IDC_CONFIG_NOWPLAYINGPATH, szBuffer, NUMOFTCHARS(szBuffer));
			Settings.strNowPlayingPath.SetTStr(szBuffer);
			GetDlgItemText(hwndDlg, IDC_CONFIG_XMLEXPORTPATH, szBuffer, NUMOFTCHARS(szBuffer));
			Settings.strXmlExportPath.SetTStr(szBuffer);
			GetDlgItemText(hwndDlg, IDC_CONFIG_HTMLEXPORTPATH, szBuffer, NUMOFTCHARS(szBuffer));
			Settings.strHtmlExportPath.SetTStr(szBuffer);
			
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;
		}

		case IDCANCEL :
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
			bReturn = TRUE;
			break;

		case IDC_CONFIG_BROWSE_NP :
			GetSaveFilePath(&(Settings.strNowPlayingPath), hwndDlg, IDC_CONFIG_NOWPLAYINGPATH, L"XML Files (*.xml)\0*.xml\0All Files (*.*)\0*.*\0");
			break;
		
		case IDC_CONFIG_BROWSE_XML :
			GetSaveFilePath(&(Settings.strXmlExportPath), hwndDlg, IDC_CONFIG_XMLEXPORTPATH, L"XML Files (*.xml)\0*.xml\0All Files (*.*)\0*.*\0");		
			break;

		case IDC_CONFIG_BROWSE_HTML :
			GetSaveFilePath(&(Settings.strHtmlExportPath), hwndDlg, IDC_CONFIG_HTMLEXPORTPATH, L"XHTML Files (*.xhtml)\0*.xhtml\0All Files (*.*)\0*.*\0");
			break;

		default :			
			break; // default

		} // switch (wParam)
		break;
	} // WM_COMMAND

	} // switch (uMsg)

	return bReturn;
}

void GetSaveFilePath(QString* strOut, HWND hwndDlg, int nIdDlgItem, WCHAR* szFilter)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	WCHAR szFileName[MAX_PATH] = L"";			

	wcscpy_s(szFileName, MAX_PATH, strOut->GetUnicode());

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hwndPlayer;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szFileName;
	ofn.lpstrDefExt = L"xml";
	ofn.nMaxFile = MAX_PATH;			

	ofn.Flags = OFN_DONTADDTORECENT | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;			

	if (GetSaveFileName(&ofn))
	{
		strOut->SetUnicode(ofn.lpstrFile);
		SetDlgItemText(hwndDlg, nIdDlgItem, ofn.lpstrFile);
	}
}