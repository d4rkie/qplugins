#include <TCHAR.h>

#include "resource.h"
#include "ABRepeatDLL.h"
#include "MainDialog.h"

//-----------------------------------------------------------------------------

void CreateMainDlg()
{
	hwndMainDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAINDLG), hwndPlayer, (DLGPROC)cbMainDlg);
	if (!hwndMainDlg) {
		MessageBox(hwndPlayer, _T("Failed to create main dialog"), _T("A-B Repeat"), MB_ICONEXCLAMATION);
		return;
	}

	// Move window
	WINDOWPLACEMENT wpPlayer, wpDlg;
	wpPlayer.length	= sizeof(WINDOWPLACEMENT);
	wpDlg.length	= sizeof(WINDOWPLACEMENT);

	GetWindowPlacement(hwndPlayer, &wpPlayer);
	GetWindowPlacement(hwndMainDlg, &wpDlg);

	LONG iWidth  = wpDlg.rcNormalPosition.right  - wpDlg.rcNormalPosition.left;
	LONG iHeight = wpDlg.rcNormalPosition.bottom - wpDlg.rcNormalPosition.top;

	// Horizontal position
	if (wpPlayer.rcNormalPosition.left - iWidth < 0) { // Place right
		wpDlg.rcNormalPosition.left		= wpPlayer.rcNormalPosition.right + 1;
		wpDlg.rcNormalPosition.right	= wpDlg.rcNormalPosition.left + iWidth;
	}
	else { // Place left
		wpDlg.rcNormalPosition.right	= wpPlayer.rcNormalPosition.left - 1;
		wpDlg.rcNormalPosition.left		= wpDlg.rcNormalPosition.right - iWidth;
	}
	// Vertical position
	wpDlg.rcNormalPosition.top    = wpPlayer.rcNormalPosition.top + 1;
	wpDlg.rcNormalPosition.bottom = wpDlg.rcNormalPosition.top + iHeight + 1;

	SetWindowPlacement(hwndMainDlg, &wpDlg);
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CALLBACKS
//-----------------------------------------------------------------------------

BOOL CALLBACK cbMainDlg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG : {
		m_bRunning = TRUE;
		m_Status = STATUS_NOMARK;

		return TRUE;	// In response to a WM_INITDIALOG message, the dialog box procedure should return zero if it calls the SetFocus function to set the focus to one of the controls in the dialog box. Otherwise, it should return nonzero, in which case the system sets the focus to the first control in the dialog box that can be given the focus.
		break;
	}
	case WM_CLOSE :
		DestroyWindow(hwndMainDlg);
		hwndMainDlg = NULL;
		m_bRunning = FALSE;
		break;

	case WM_COMMAND : 
		switch (LOWORD(wParam))
		{
		case IDC_BTN_MARK : {
			const static size_t STR_MARK_SIZE = 256;
			_TCHAR strMark[STR_MARK_SIZE];
			
			switch (m_Status)
			{
			case STATUS_NOMARK : {
				if (hwndQCDWA == NULL)
					hwndQCDWA = FindWindow("Winamp v1.x", NULL);
				m_iStartPos = SendMessage(hwndQCDWA, WM_USER, 0, IPC_GETOUTPUTTIME);

				FormatTime(strMark, STR_MARK_SIZE, m_iStartPos);
				SetDlgItemText(hwndDlg, IDC_EDIT_STARTPOS, strMark);
				SetDlgItemText(hwndDlg, IDC_BTN_MARK, "(B) Mark");
				m_Status = STATUS_MARKED;
				break;
			}
			case STATUS_MARKED : {
				m_iEndPos = SendMessage(hwndQCDWA, WM_USER, 0, IPC_GETOUTPUTTIME);

				if (m_iEndPos > m_iStartPos) {
					FormatTime(strMark, STR_MARK_SIZE, m_iEndPos);
					SetDlgItemText(hwndDlg, IDC_EDIT_ENDPOS, strMark);
					SetDlgItemText(hwndDlg, IDC_BTN_MARK, "Stop");
					m_Status = STATUS_ABMODE;
				}
				else {
					m_iStartPos = 0;
					m_iEndPos = 0;
					m_Status = STATUS_NOMARK;
					SetDlgItemText(hwndDlg, IDC_EDIT_STARTPOS, "");
					SetDlgItemText(hwndDlg, IDC_EDIT_ENDPOS, "");
					SetDlgItemText(hwndDlg, IDC_BTN_MARK, "(A) Mark");
				}
				break;
			}
			case STATUS_ABMODE : {
				SetDlgItemText(hwndDlg, IDC_EDIT_STARTPOS, "");
				SetDlgItemText(hwndDlg, IDC_EDIT_ENDPOS, "");
				SetDlgItemText(hwndDlg, IDC_BTN_MARK, "(A) Mark");
				m_Status = STATUS_NOMARK;
				break;
			}
			} // switch (m_iStatus)
			
		}
		} // switch (LOWORD(wParam))
		break;
	} // Switch (uMsg)
	return FALSE;
}

//-----------------------------------------------------------------------------

void FormatTime(_TCHAR* str, size_t ccbSize, int iTimeIn)
{
	long iMinutes, iMSeconds, iSeconds, iTmp;
	int iTime = iTimeIn / 100; // Remove 2 last decimals

	iMinutes = iTime / 600;
	iTmp = (iTime % 600);
	iSeconds = iTmp / 10;
	iMSeconds = iTmp - iSeconds * 10;

	static const size_t TMP_SIZE = 32;
	_TCHAR strTemp[TMP_SIZE];

	_itot_s(iMinutes, strTemp, TMP_SIZE, 10);
	_tcscpy_s(str, ccbSize, strTemp);
	_tcscat_s(str, ccbSize, _T(":"));

	_itot_s(iSeconds, strTemp, TMP_SIZE, 10);
	if (iSeconds < 10)
		_tcscat_s(str, ccbSize, _T("0"));
	_tcscat_s(str, ccbSize, strTemp);
	_tcscat_s(str, ccbSize, _T(":"));

	_itot_s(iMSeconds, strTemp, TMP_SIZE, 10);
	_tcscat_s(str, ccbSize, strTemp);
}