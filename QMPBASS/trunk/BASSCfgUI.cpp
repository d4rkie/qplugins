#define _WIN32_IE  0x0600

#include ".\BASSCfgUI.h"

#include "QCDBASS.h"

#include "Hyperlinks.h"
#include "bass.h"

#include <tchar.h>

const unsigned int PREAMP_RANGE = 24;
LPCTSTR resolution_table[] = { "16 bit", "24 bit", "32 bit" };
LPCTSTR noise_shaping_table[] = { "None", "Low", "Medium", "High" };
LPCTSTR replaygain_table[] = { "Disabled", "Use Track Gain", "Use Album Gain" };
BOOL firstShow;

void LoadResString(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax)
{
	ResInfo ri;
	ri.struct_size = sizeof(ResInfo);
	ri.hModule = hInstance;
	ri.resID = MAKEINTRESOURCEW(uID);
	ri.langid = 0;

	QCDCallbacks.Service(opLoadResString, lpBuffer, nBufferMax, (long)&ri);
}

LPCDLGTEMPLATE LoadResDialog(HINSTANCE hInstance, int nTemplate)
{
	ResInfo ri;
	ri.struct_size = sizeof(ResInfo);
	ri.hModule = hInstance;
	ri.resID = MAKEINTRESOURCEW(nTemplate);
	ri.langid = 0;

	return (LPCDLGTEMPLATE)QCDCallbacks.Service(opLoadResDialog, &ri, 0, 0);
}

HWND DoConfigSheet(HINSTANCE hInstance, HWND hwndParent)
{
	TCHAR buf[50];

    PROPSHEETPAGE psp[5];
    PROPSHEETHEADER psh;

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_DEFAULT | PSP_DLGINDIRECT | PSP_PREMATURE;
    psp[0].hInstance = hInstance;
    psp[0].pResource = LoadResDialog(hInstance, IDD_GENERAL);
    psp[0].pfnDlgProc = (DLGPROC)GeneralDlgProc;
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;
    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_DEFAULT | PSP_DLGINDIRECT | PSP_PREMATURE;
    psp[1].hInstance = hInstance;
    psp[1].pResource = LoadResDialog(hInstance, IDD_ADVANCED);
    psp[1].pfnDlgProc = (DLGPROC)AdvancedDlgProc;
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;
    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_DEFAULT | PSP_DLGINDIRECT | PSP_PREMATURE;
    psp[2].hInstance = hInstance;
	psp[2].pResource = LoadResDialog(hInstance, IDD_STREAMING);
    psp[2].pfnDlgProc = (DLGPROC)StreamingDlgProc;
    psp[2].lParam = 0;
    psp[2].pfnCallback = NULL;
    psp[3].dwSize = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags = PSP_DEFAULT | PSP_DLGINDIRECT | PSP_PREMATURE;
    psp[3].hInstance = hInstance;
	psp[3].pResource = LoadResDialog(hInstance, IDD_STREAM_SAVING);
    psp[3].pfnDlgProc = (DLGPROC)StreamSavingDlgProc;
    psp[3].lParam = 0;
    psp[3].pfnCallback = NULL;
    psp[4].dwSize = sizeof(PROPSHEETPAGE);
    psp[4].dwFlags = PSP_DEFAULT | PSP_DLGINDIRECT | PSP_PREMATURE;
    psp[4].hInstance = hInstance;
	psp[4].pResource = LoadResDialog(hInstance, IDD_ADDONS);
    psp[4].pfnDlgProc = (DLGPROC)AddonsDlgProc;
    psp[4].lParam = 0;
    psp[4].pfnCallback = NULL;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_DEFAULT | PSH_MODELESS | PSH_PROPSHEETPAGE | PSH_USECALLBACK;
    psh.hwndParent = hwndParent;
    psh.hInstance = hInstance;
	LoadResString(hInstance, IDS_CONFIG_SHEET_TITLE, buf, sizeof(buf));
    psh.pszCaption = buf;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = uPrefPage;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = (PFNPROPSHEETCALLBACK)ConfigSheetProc;

	return (HWND)PropertySheet(&psh);
}

WNDPROC oldProc;
LRESULT CALLBACK NewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)) {
		LRESULT lRet;

		uPrefPage = PropSheet_HwndToIndex(hwnd, PropSheet_GetCurrentPageHwnd(hwnd));

		lRet = CallWindowProc(oldProc, hwnd, uMsg, wParam, lParam);

		if ( !PropSheet_GetCurrentPageHwnd(hwnd)) {
			RECT rc;
			GetWindowRect(hwndConfig, &rc);
			xPrefPos = rc.left;
			yPrefPos = rc.top;

			DestroyWindow(hwndConfig);
			hwndConfig = NULL;
		}

		return lRet;
	}

	if ( uMsg == WM_SYSCOMMAND && (wParam & 0xFFF0) == SC_CLOSE) {
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		return 0L;
	}

	// set property sheet first, do it only once
	if ( uMsg == WM_SIZE) {
		SetWindowPos( hwnd, NULL, xPrefPos, yPrefPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		firstShow = FALSE;
	}

    return CallWindowProc(oldProc, hwnd, uMsg, wParam, lParam);
}
int CALLBACK ConfigSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
	if (uMsg == PSCB_INITIALIZED) {
		firstShow = TRUE;

		oldProc = (WNDPROC)SetWindowLongPtr( hwndDlg, GWLP_WNDPROC, (LONG_PTR)NewProc);
	}

	return 0;
}

INT_PTR CALLBACK GeneralDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			int i;
			LPTSTR p;

			SetDlgItemText(hwndDlg, IDC_EXTENSIONS, strExtensions);

			// init device list
			SendDlgItemMessage(hwndDlg, IDC_DEVICE, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_DEVICE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Decoding")); // add decoding first
			p = NULL;
			for ( i = 1; p = (LPTSTR)BASS_GetDeviceDescription(i); ++i) {
				SendDlgItemMessage(hwndDlg, IDC_DEVICE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)p);
			}
			SendDlgItemMessage(hwndDlg, IDC_DEVICE, CB_SETCURSEL, uDeviceNum, 0);

			// init resolution list
			SendDlgItemMessage(hwndDlg, IDC_RESOLUTION, CB_RESETCONTENT, 0, 0);
			for ( i = 0; i < sizeof(resolution_table)/sizeof(resolution_table[0]); ++i) {
				SendDlgItemMessage(hwndDlg, IDC_RESOLUTION, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)resolution_table[i]);
			}
			SendDlgItemMessage(hwndDlg, IDC_RESOLUTION, CB_SETCURSEL, (uResolution / 8 - 2), 0);

			// init dither controls
			CheckDlgButton( hwndDlg, IDC_DITHER, !!bDither);
			for ( i = 0; i < sizeof(noise_shaping_table)/sizeof(noise_shaping_table[0]); ++i)
				SendDlgItemMessage(hwndDlg, IDC_NOISE_SHAPING, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)noise_shaping_table[i]);
			SendDlgItemMessage(hwndDlg, IDC_NOISE_SHAPING, CB_SETCURSEL, uNoiseShaping, 0);
			EnableWindow( GetDlgItem( hwndDlg, IDC_DITHER), uResolution < 24);
			ShowWindow( GetDlgItem( hwndDlg, IDC_DITHER), uResolution > 24 ? SW_HIDE : SW_SHOW);
			EnableWindow( GetDlgItem( hwndDlg, IDC_NOISE_SHAPING), bDither);
			ShowWindow( GetDlgItem( hwndDlg, IDC_NOISE_SHAPING), uResolution > 24 ? SW_HIDE : SW_SHOW);

			// init priority controls
			SendDlgItemMessage(hwndDlg, IDC_PRIORITY, TBM_SETRANGE, TRUE, MAKELONG(0, 3));
			SendDlgItemMessage(hwndDlg, IDC_PRIORITY, TBM_SETPOS, TRUE, 3-uPriority);

			// init misc controls
			CheckDlgButton(hwndDlg, IDC_EQENABLED, bEqEnabled);
			CheckDlgButton(hwndDlg, IDC_SHOW_AVG_BITRATE, bShowVBR);
		}

		return TRUE;
	case WM_VSCROLL:
		{
			PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
		}

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_EXTENSIONS:
			{
				int len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_EXTENSIONS)) + 1;
				LPTSTR lpstr = new TCHAR[len];
				ZeroMemory(lpstr, len);
				GetDlgItemText(hwndDlg, IDC_EXTENSIONS, lpstr, len);
				if (lstrcmpi(lpstr, strExtensions))
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		case IDC_DEVICE:
		case IDC_NOISE_SHAPING:
			{
				if (CBN_SELCHANGE == HIWORD(wParam))
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		case IDC_RESOLUTION:
			{
				if ( CBN_SELCHANGE == HIWORD(wParam)) {
					UINT res = 8 * ( 2 + SendDlgItemMessage( hwndDlg, IDC_RESOLUTION, CB_GETCURSEL, 0, 0));
					if ( res > 24)
						CheckDlgButton( hwndDlg, IDC_DITHER, FALSE); // disable dither for 32bit!
					else if ( res < 24)
						CheckDlgButton( hwndDlg, IDC_DITHER, !!bDither);
					else
						CheckDlgButton( hwndDlg, IDC_DITHER, TRUE); // enable dither for 24bit always
					EnableWindow( GetDlgItem( hwndDlg, IDC_DITHER), res < 24);
					ShowWindow( GetDlgItem( hwndDlg, IDC_DITHER), res > 24 ? SW_HIDE : SW_SHOW);
					EnableWindow( GetDlgItem( hwndDlg, IDC_NOISE_SHAPING), res == 24 ? TRUE : bDither);
					ShowWindow( GetDlgItem( hwndDlg, IDC_NOISE_SHAPING), res > 24 ? SW_HIDE : SW_SHOW);

					PropSheet_Changed(GetParent( hwndDlg), hwndDlg);
				}
			}

			break;
		case IDC_DITHER:
			{
				BOOL ck = IsDlgButtonChecked(hwndDlg, IDC_DITHER);
				EnableWindow( GetDlgItem( hwndDlg, IDC_NOISE_SHAPING), ck);
				if ( bDither != ck )
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		case IDC_EQENABLED:
			{
				if ( bEqEnabled != (int)IsDlgButtonChecked(hwndDlg, IDC_EQENABLED) )
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		case IDC_SHOW_AVG_BITRATE:
			{
				if ( bShowVBR != (int)IsDlgButtonChecked(hwndDlg, IDC_SHOW_AVG_BITRATE) )
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		case IDC_GET_INFO:
			{
				BASS_INFO info;
				if (BASS_GetInfo(&info)) {
					TCHAR buf1[2*MAX_PATH], buf2[2*MAX_PATH];
					TCHAR buf3[10], buf4[10], buf5[10];

					LoadResString(hInstance, IDS_DEVICE_INFO, buf2, sizeof(buf2));
					LoadResString(hInstance, IDS_DEVICE_SUPPORT_YES, buf3, sizeof(buf3));
					LoadResString(hInstance, IDS_DEVICE_SUPPORT_NO, buf4, sizeof(buf4));
					LoadResString(hInstance, IDS_DEVICE_UNKNOWN, buf5, sizeof(buf5));

                    wsprintf(buf1, buf2, 
						info.hwsize, 
						info.hwfree, 
						info.freesam, 
						info.free3d, 
						info.minrate, info.maxrate, 
						info.eax ? buf3 : buf4, 
						info.minbuf, 
						info.dsver, 
						info.latency, 
						info.speakers, 
						info.driver ? info.driver : buf5);
					LoadResString(hInstance, IDS_DEVICE_INFO_TITLE, buf2, sizeof(buf2));
					MessageBox(hwndDlg, buf1, buf2, MB_OK | MB_ICONINFORMATION);
				} else {
					TCHAR buf1[50], buf2[10];
					LoadResString(hInstance, IDS_DEVICE_INFO_ERROR, buf1, sizeof(buf1));
					LoadResString(hInstance, IDS_ERROR_TITLE, buf2, sizeof(buf2));
					MessageBox(hwndDlg, buf1, buf2, MB_OK | MB_ICONSTOP);
				}
			}

			break;
		}

		return TRUE;
	case WM_NOTIFY:
		{
			LPPSHNOTIFY lppsn = (LPPSHNOTIFY)lParam;
			switch (lppsn->hdr.code)
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(hwndDlg, DWL_MSGRESULT, FALSE);
				}

				break;
			case PSN_APPLY:
				{
					uDeviceNum		= SendDlgItemMessage(hwndDlg, IDC_DEVICE, CB_GETCURSEL, 0, 0);
					uResolution		= 8 * (2 + SendDlgItemMessage(hwndDlg, IDC_RESOLUTION, CB_GETCURSEL, 0, 0));
					bDither			= IsDlgButtonChecked(hwndDlg, IDC_DITHER);
					uNoiseShaping	= SendDlgItemMessage(hwndDlg, IDC_NOISE_SHAPING, CB_GETCURSEL, 0, 0);
					uPriority		= 3-SendDlgItemMessage(hwndDlg, IDC_PRIORITY, TBM_GETPOS, 0, 0);
					bEqEnabled		= IsDlgButtonChecked(hwndDlg, IDC_EQENABLED);
					bShowVBR		= IsDlgButtonChecked(hwndDlg, IDC_SHOW_AVG_BITRATE);

					GetDlgItemText(hwndDlg, IDC_EXTENSIONS, (LPTSTR)(LPCTSTR)strExtensions, MAX_PATH);
					listExtensions.clear();

					if ( lppsn->lParam != TRUE) // just press Apply button
						SetWindowLong(hwndDlg, DWL_MSGRESULT, PSNRET_NOERROR);
				}

				break;
			}
		}

		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK AdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			int i;
			TCHAR buffer[10];

			uFadeIn = uFadeIn < 0 ? 0 : (uFadeIn > 10000 ? 10000 : uFadeIn); // value fixer
			SetDlgItemInt(hwndDlg, IDC_FADE_IN, uFadeIn, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_FADE_IN, EM_SETLIMITTEXT, 5*sizeof(TCHAR), 0);
			uFadeOut = uFadeOut < 0 ? 0 : (uFadeOut > 10000 ? 10000 : uFadeOut); // value fixer
			SetDlgItemInt(hwndDlg, IDC_FADE_OUT, uFadeOut, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_FADE_OUT, EM_SETLIMITTEXT, 5*sizeof(TCHAR), 0);

			UDACCEL uda = {1, 100};
			SendDlgItemMessage(hwndDlg, IDC_FADE_IN_UD, UDM_SETACCEL, 1, (LPARAM)(LPUDACCEL)&uda);
			SendDlgItemMessage(hwndDlg, IDC_FADE_IN_UD, UDM_SETRANGE, 0, (LPARAM)MAKELONG(10000, 0));
			SendDlgItemMessage(hwndDlg, IDC_FADE_OUT_UD, UDM_SETACCEL, 1, (LPARAM)(LPUDACCEL)&uda);
			SendDlgItemMessage(hwndDlg, IDC_FADE_OUT_UD, UDM_SETRANGE, 0, (LPARAM)MAKELONG(10000, 0));

			// Preamp
			SendDlgItemMessage(hwndDlg, IDC_PREAMP, TBM_SETRANGE, TRUE, MAKELONG(0, PREAMP_RANGE*2));
			SendDlgItemMessage(hwndDlg, IDC_PREAMP, TBM_SETPOS, TRUE, nPreAmp+PREAMP_RANGE);
			_stprintf(buffer, "%+2d dB", (int)nPreAmp);
			SetWindowText(GetDlgItem(hwndDlg, IDC_PA), buffer);

			// RG Preamp
			SendDlgItemMessage(hwndDlg, IDC_RG_PREAMP, TBM_SETRANGE, TRUE, MAKELONG(0, PREAMP_RANGE*2));
			SendDlgItemMessage(hwndDlg, IDC_RG_PREAMP, TBM_SETPOS, TRUE, nRGPreAmp+PREAMP_RANGE);
			_stprintf(buffer, "%+2d dB", (int)nRGPreAmp);
			SetWindowText(GetDlgItem(hwndDlg, IDC_RG_PA), buffer);

			CheckDlgButton(hwndDlg, IDC_HARD_LIMITER, !!bHardLimiter);
			for (i = 0; i < sizeof(replaygain_table)/sizeof(replaygain_table[0]); i++)
				SendDlgItemMessage(hwndDlg, IDC_REPLAYGAIN_MODE, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)replaygain_table[i]);
			SendDlgItemMessage(hwndDlg, IDC_REPLAYGAIN_MODE, CB_SETCURSEL, uReplayGainMode, 0);
		}

		return TRUE;
	case WM_HSCROLL:
		{
			int nPos;
			TCHAR buffer[10];
			
			nPos = SendDlgItemMessage(hwndDlg, IDC_PREAMP, TBM_GETPOS, 0, 0) - PREAMP_RANGE;			
			_stprintf(buffer, "%+2d dB", nPos);
			SetDlgItemText(hwndDlg, IDC_PA, buffer);

			nPos = SendDlgItemMessage(hwndDlg, IDC_RG_PREAMP, TBM_GETPOS, 0, 0) - PREAMP_RANGE;			
			_stprintf(buffer, "%+2d dB", nPos);
			SetDlgItemText(hwndDlg, IDC_RG_PA, buffer);

			PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
		}

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_FADE_IN:
			{
				if (uFadeIn != (int)GetDlgItemInt(hwndDlg, IDC_FADE_IN, NULL, FALSE))
                    PropSheet_Changed(GetParent(hwndDlg), hwndDlg);					
			}

			break;
		case IDC_FADE_OUT:
			{
				if (uFadeOut != (int)GetDlgItemInt(hwndDlg, IDC_FADE_OUT, NULL, FALSE))
                    PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		case IDC_HARD_LIMITER:
			{
				if ( bHardLimiter != (int)IsDlgButtonChecked(hwndDlg, IDC_HARD_LIMITER) )
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}
			
			break;
		case IDC_REPLAYGAIN_MODE:
			{
				if (CBN_SELCHANGE == HIWORD(wParam))
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		}

		return TRUE;
	case WM_NOTIFY:
		{
			LPPSHNOTIFY lppsn = (LPPSHNOTIFY)lParam;
			switch (lppsn->hdr.code)
			{
			case PSN_KILLACTIVE:
				{
					UINT tmp_in = GetDlgItemInt(hwndDlg, IDC_FADE_IN, NULL, FALSE);
					UINT tmp_out = GetDlgItemInt(hwndDlg, IDC_FADE_OUT, NULL, FALSE);
					if(tmp_in < 0 || tmp_in > 10000 || tmp_out < 0 || tmp_out > 10000) {
						TCHAR buf1[100], buf2[10];

						LoadResString(hInstance, IDS_INVALID_VALUE1, buf1, sizeof(buf1));
						LoadResString(hInstance, IDS_ERROR_TITLE, buf2, sizeof(buf2));
						MessageBox(hwndDlg, buf1, buf2, MB_OK | MB_ICONERROR);

						if (tmp_in < 0 || tmp_in > 10000)
							SetDlgItemInt(hwndDlg, IDC_FADE_IN, uFadeIn, FALSE);
						if (tmp_out < 0 || tmp_out > 10000)
							SetDlgItemInt(hwndDlg, IDC_FADE_OUT, uFadeOut, FALSE);

						SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
					}
					else
						SetWindowLong(hwndDlg, DWL_MSGRESULT, FALSE);
				}

				break;
			case PSN_APPLY:
				{
					BOOL ret;
					UINT tmp;
					tmp = GetDlgItemInt(hwndDlg, IDC_FADE_IN, &ret, FALSE);
					if(ret)
						uFadeIn = tmp;
					tmp = GetDlgItemInt(hwndDlg, IDC_FADE_OUT, &ret, FALSE);
					if(ret)
						uFadeOut = tmp;

					nPreAmp			= SendDlgItemMessage(hwndDlg, IDC_PREAMP, TBM_GETPOS, 0, 0) - PREAMP_RANGE;
					nRGPreAmp		= SendDlgItemMessage(hwndDlg, IDC_RG_PREAMP, TBM_GETPOS, 0, 0) - PREAMP_RANGE;
					uReplayGainMode	= SendDlgItemMessage(hwndDlg, IDC_REPLAYGAIN_MODE, CB_GETCURSEL, 0, 0);
					bHardLimiter	= IsDlgButtonChecked(hwndDlg, IDC_HARD_LIMITER);

					if ( lppsn->lParam != TRUE) // just press Apply button
						SetWindowLong(hwndDlg, DWL_MSGRESULT, PSNRET_NOERROR);
				}

				break;
			}
		}

		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK StreamingDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			uBufferLen  = uBufferLen < 5 ? 5 : (uBufferLen > 60 ? 60 : uBufferLen); // value fixer
			SetDlgItemInt(hwndDlg, IDC_BUFFER_LENGTH, uBufferLen, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_BUFFER_LENGTH, EM_SETLIMITTEXT, 2*sizeof(char), 0);

			SendDlgItemMessage(hwndDlg, IDC_BUFFER_LENGTH_UD, UDM_SETRANGE, 0, (LPARAM)MAKELONG(60, 5));

            CheckDlgButton(hwndDlg, IDC_ENABLE_STREAM_TITLE, bStreamTitle);
		}

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUFFER_LENGTH:
			{
				if(uBufferLen != (int)GetDlgItemInt(hwndDlg, IDC_BUFFER_LENGTH, NULL, FALSE))
					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		case IDC_ENABLE_STREAM_TITLE:
			{
				PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		}

		return TRUE;
	case WM_NOTIFY:
		{
			LPPSHNOTIFY lppsn = (LPPSHNOTIFY)lParam;
			switch (lppsn->hdr.code)
			{
			case PSN_KILLACTIVE:
				{
					UINT tmp = GetDlgItemInt(hwndDlg, IDC_BUFFER_LENGTH, NULL, FALSE);
					if(tmp < 5 || tmp > 60) {
						TCHAR buf1[100], buf2[10];

						LoadResString(hInstance, IDS_INVALID_VALUE2, buf1, sizeof(buf1));
						LoadResString(hInstance, IDS_ERROR_TITLE, buf2, sizeof(buf2));
						MessageBox(hwndDlg, buf1, buf2, MB_OK | MB_ICONERROR);

						SetDlgItemInt(hwndDlg, IDC_BUFFER_LENGTH, uBufferLen, FALSE);

						SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
					}
					else
						SetWindowLong(hwndDlg, DWL_MSGRESULT, FALSE);
				}

				break;
			case PSN_APPLY:
				{
					BOOL ret;
					UINT tmp;
					tmp = GetDlgItemInt(hwndDlg, IDC_BUFFER_LENGTH, &ret, FALSE);
					if(ret)
						uBufferLen = tmp;

					bStreamTitle = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_STREAM_TITLE);

					if ( lppsn->lParam != TRUE) // just press Apply button
						SetWindowLong(hwndDlg, DWL_MSGRESULT, PSNRET_NOERROR);
				}

				break;
			}
		}

		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK StreamSavingDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			CheckDlgButton(hwndDlg, IDC_ENABLE_STREAM_SAVING, bStreamSaving);
			CheckDlgButton(hwndDlg, IDC_AUTOSHOW_STREAM_SAVING_BAR, bAutoShowStreamSavingBar);

			if ( !strStreamSavingPath.is_empty() && PathFileExists(strStreamSavingPath) ) {
					SetDlgItemText(hwndDlg, IDC_STREAM_SAVING_PATH, strStreamSavingPath);
			}

			CheckDlgButton(hwndDlg, IDC_SAVE_STREAMS_BASED_ON_TITLE, bSaveStreamsBasedOnTitle);
		}

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_ENABLE_STREAM_SAVING:
		case IDC_AUTOSHOW_STREAM_SAVING_BAR:
		case IDC_SAVE_STREAMS_BASED_ON_TITLE:
			{
				PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
			}

			break;
		case IDC_STREAM_SAVING_PATH:
			{
				TCHAR buf[50];
				TCHAR szBuffer[MAX_PATH];

				LoadResString(hInstance, IDS_SAVING_FOLDER_TITLE, buf, sizeof(buf));
				if (browse_folder(szBuffer, buf, hwndDlg) && lstrcmpi(strStreamSavingPath, szBuffer) ) {
					SetDlgItemText(hwndDlg, IDC_STREAM_SAVING_PATH, szBuffer);

					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
				}
			}

			break;
		}

		return TRUE;
	case WM_NOTIFY:
		{
			LPPSHNOTIFY lppsn = (LPPSHNOTIFY)lParam;
			switch (lppsn->hdr.code)
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(hwndDlg, DWL_MSGRESULT, FALSE);
				}

				break;
			case PSN_APPLY:
				{
					bStreamSaving = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_STREAM_SAVING);
					bAutoShowStreamSavingBar = IsDlgButtonChecked(hwndDlg, IDC_AUTOSHOW_STREAM_SAVING_BAR);

					TCHAR szBuffer[MAX_PATH];
					GetDlgItemText(hwndDlg, IDC_STREAM_SAVING_PATH, szBuffer, MAX_PATH);
					if (PathFileExists(szBuffer)) {
						lstrcpy((LPTSTR)(LPCTSTR)strStreamSavingPath, szBuffer);
					}

					if (hwndStreamSavingBar)
						SendMessage(hwndStreamSavingBar, WM_INITDIALOG, 0, 0);

					reset_menu();

					bSaveStreamsBasedOnTitle = IsDlgButtonChecked(hwndDlg, IDC_SAVE_STREAMS_BASED_ON_TITLE);

					if ( lppsn->lParam != TRUE) // just press Apply button
						SetWindowLong(hwndDlg, DWL_MSGRESULT, PSNRET_NOERROR);
				}

				break;
			}
		}

		return TRUE;
	}

	return FALSE;
}


INT_PTR CALLBACK AddonsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			if ( !strAddonsDir.is_empty())
				SetDlgItemText(hwndDlg, IDC_ADDONS_DIR, strAddonsDir);

			SendDlgItemMessage(hwndDlg, IDC_ADDONS_LIST, LB_RESETCONTENT, 0, 0);
			for ( list<string>::iterator it = listAddons.begin(); it != listAddons.end(); it++) {
				SendDlgItemMessage(hwndDlg, IDC_ADDONS_LIST, LB_ADDSTRING, 0, (LPARAM)(*it).c_str());
			}

			SetWindowLong(GetDlgItem(hwndDlg, IDC_ADDONS_LINK), GWL_USERDATA, (long)_T("http://www.un4seen.com/bass.html#addons"));
			ConvertStaticToHyperlink(hwndDlg, IDC_ADDONS_LINK);
		}

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_ADDONS_DIR:
			{
				TCHAR buf[50];
				TCHAR szBuffer[MAX_PATH];

				LoadResString(hInstance, IDS_ADDONS_FOLDER_TITLE, buf, sizeof(buf));
				if (browse_folder(szBuffer, buf, hwndDlg) && lstrcmpi(strAddonsDir, szBuffer) ) {
					SetDlgItemText(hwndDlg, IDC_ADDONS_DIR, szBuffer);

					PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
				}
			}

			break;
		}

		return TRUE;
	case WM_NOTIFY:
		{
			LPPSHNOTIFY lppsn = (LPPSHNOTIFY)lParam;
			switch (lppsn->hdr.code)
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(hwndDlg, DWL_MSGRESULT, FALSE);
				}

				break;
			case PSN_APPLY:
				{
					TCHAR szBuffer[MAX_PATH];
					GetDlgItemText(hwndDlg, IDC_ADDONS_DIR, szBuffer, MAX_PATH);
					if (PathFileExists(szBuffer) && lstrcmpi( szBuffer, strAddonsDir)) {
						TCHAR buf1[200], buf2[20];
						LoadResString(hInstance, IDS_ADDONS_WARNING, buf1, sizeof(buf1));
						LoadResString(hInstance, IDS_ADDONS_WARNING_TITLE, buf2, sizeof(buf2));
						MessageBox(hwndDlg, buf1, buf2, MB_OK | MB_ICONINFORMATION);
 
						lstrcpy((LPTSTR)(LPCTSTR)strAddonsDir, szBuffer);
					}

					if ( lppsn->lParam != TRUE) // just press Apply button
						SetWindowLong(hwndDlg, DWL_MSGRESULT, PSNRET_NOERROR);
				}

				break;
			}
		}

		return TRUE;
	}

	return FALSE;
}

//-- for About dialog box
HWND DoAboutDlg(HINSTANCE hInstance, HWND hwndParent)
{
	HWND ret = CreateDialogIndirect(hInstance, LoadResDialog(hInstance, IDD_ABOUT), hwndParent, (DLGPROC)AboutDlgProc);
	ShowWindow(ret, SW_SHOW);

	return ret;
}

INT_PTR CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			DWORD ver = BASS_GetVersion();
            TCHAR buf[10];
			_stprintf( buf, _T("v%d.%d.%d.%d"), HIBYTE(HIWORD(ver)), LOBYTE(HIWORD(ver)), HIBYTE(LOWORD(ver)), HIBYTE(LOWORD(ver)));
			SetDlgItemText(hwndDlg, IDC_BASS_VERSION, buf);

			SetDlgItemText(hwndDlg, IDC_PLUGIN_VERSION, PLUGIN_VERSION);
		}

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				DestroyWindow(hwndAbout);
				hwndAbout = NULL;
			}
			break;
		}

		return TRUE;
	}

	return FALSE;
}


//-- for stream saving bar
HWND DoStreamSavingBar(HINSTANCE hInstance, HWND hwndParent)
{
	HWND ret = CreateDialogIndirect(hInstance, LoadResDialog(hInstance, IDD_STREAM_SAVING_BAR), hwndParent, (DLGPROC)StreamSavingBarProc);
	SetWindowPos(ret, HWND_TOPMOST, xStreamSavingBar, yStreamSavingBar, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

	return ret;
}

static HWND hwndToolTip = NULL;
static char szToolTip[3*MAX_PATH] = {'\0'};
static HIMAGELIST himlStreamSavingBar = NULL;
#define DrawBarButton(i) ImageList_Draw(himlStreamSavingBar, (i), lpdis->hDC, 0, 0, ILD_NORMAL)
INT_PTR CALLBACK StreamSavingBarProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpdis;

	switch (uMsg) 
	{
	case WM_NCHITTEST:
		{
			RECT rc;
			POINT pt = {LOWORD(lParam), HIWORD(lParam)};
			GetWindowRect(hwndDlg, &rc);
			if (PtInRect(&rc, pt))
				SetWindowLong(hwndDlg, DWL_MSGRESULT, HTCAPTION);
		}

		return TRUE;
	case WM_INITDIALOG:
		{
			TCHAR buf1[20], buf2[20];

			LoadResString(hInstance, IDS_STREAM_SAVING_ON, buf1, sizeof(buf1));
			LoadResString(hInstance, IDS_STREAM_SAVING_OFF, buf2, sizeof(buf2));
			// fix our size and pos for better UI
			// enlarge the whole bar
			SetWindowPos(hwndDlg, 0, 0, 0, 268, 39, SWP_NOMOVE | SWP_NOZORDER);
			// enlarge individual control
			SetWindowPos(GetDlgItem(hwndDlg, IDC_STREAM_SAVING_BAR_GO), 0, 6, 6, 18, 18, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hwndDlg, IDC_STREAM_SAVING_BAR_SKIP_TITLE), 0, 26, 6, 18, 18, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hwndDlg, IDC_STREAM_SAVING_BAR_STATUS), 0, 50, 6, 140, 18, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hwndDlg, IDC_STREAM_SAVING_BAR_SETPATH), 0, 194, 6, 18, 18, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hwndDlg, IDC_STREAM_SAVING_BAR_BROWSE), 0, 214, 6, 18, 18, SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hwndDlg, IDC_STREAM_SAVING_BAR_CLOSE), 0, 234, 6, 18, 18, SWP_NOZORDER);

			EnableWindow(GetDlgItem(hwndDlg, IDC_STREAM_SAVING_BAR_SKIP_TITLE), bStreamSaving);
			SetDlgItemText(hwndDlg, IDC_STREAM_SAVING_BAR_STATUS, bStreamSaving ? buf1 :buf2);

			// load image list
			if (!himlStreamSavingBar)
				himlStreamSavingBar = ImageList_LoadImage(hInstance, MAKEINTRESOURCE(IDB_STREAM_SAVING_BAR), 18, 0, CLR_NONE, IMAGE_BITMAP, LR_CREATEDIBSECTION);

			// create tooltips
			INITCOMMONCONTROLSEX icex;
			// Load the ToolTip class from the DLL.
			icex.dwSize = sizeof(icex);
			icex.dwICC  = ICC_BAR_CLASSES;
			if(InitCommonControlsEx(&icex) && !hwndToolTip) {
				// Create the ToolTip control.
				hwndToolTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, 
					WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
					CW_USEDEFAULT, CW_USEDEFAULT, 
					CW_USEDEFAULT, CW_USEDEFAULT, 
					hwndDlg, (HMENU)NULL, hInstance, 
					NULL);
				SetWindowPos(hwndToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

				// Prepare TOOLINFO structure.
				TOOLINFO ti;
				ti.cbSize		= sizeof(TOOLINFO);
				ti.uFlags		= TTF_SUBCLASS;
				ti.hwnd			= hwndDlg;
				ti.uId			= IDC_STREAM_SAVING_BAR_STATUS;
				ti.hinst		= hInstance;
				ti.lpszText		= LPSTR_TEXTCALLBACK;
				ti.rect.left	= 50;
				ti.rect.top		= 6;
				ti.rect.right	= 190;
				ti.rect.bottom	= 24;
				SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
			}
		}

		return TRUE;
	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if (lpnmhdr->code == TTN_GETDISPINFO) {
				LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lpnmhdr;

				switch (lpnmhdr->idFrom)
				{
				case IDC_STREAM_SAVING_BAR_STATUS:
					{
						SendMessage(lpnmhdr->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 3*MAX_PATH);

						//if (bStreamSaving && cur_title && *cur_title) {
						if (bStreamSaving && decoderInfo.pDecoder->m_strCurTitle &&
							*decoderInfo.pDecoder->m_strCurTitle)
						{
							TCHAR buf[50];
							LoadResString(hInstance, IDS_STREAM_SAVING_TOOLTIP, buf, sizeof(buf));
							wsprintf(szToolTip, buf, decoderInfo.pDecoder->m_strCurTitle, strStreamSavingPath);
						} else {
							TCHAR buf[50];
							LoadResString(hInstance, IDS_STREAM_SAVING_TOOLTIP_STATUS, buf, sizeof(buf));
							lstrcpy(szToolTip, buf);
						}

						lpttd->lpszText = szToolTip;
					}

					break;
				}
			}
		}

		return TRUE;
	case WM_CTLCOLORDLG: // reset the background color of dialog box and static control to white
	case WM_CTLCOLORSTATIC:
		return (INT_PTR)GetStockObject(WHITE_BRUSH);

	case WM_DRAWITEM:
		{
			lpdis = (LPDRAWITEMSTRUCT)lParam;

			// Destination
			switch(lpdis->CtlID)
			{
			case IDC_STREAM_SAVING_BAR_GO:
				DrawBarButton( bStreamSaving ? (lpdis->itemState&ODS_SELECTED?1:0) : (lpdis->itemState&ODS_SELECTED?6:5) );
				break;
			case IDC_STREAM_SAVING_BAR_SKIP_TITLE:
				DrawBarButton( bStreamSaving ? (lpdis->itemState&ODS_SELECTED?3:2) : 4);
				break;
			case IDC_STREAM_SAVING_BAR_BROWSE:
				DrawBarButton(lpdis->itemState&ODS_SELECTED?8:7);
				break;
			case IDC_STREAM_SAVING_BAR_SETPATH:
				DrawBarButton(lpdis->itemState&ODS_SELECTED?10:9);
				break;
			case IDC_STREAM_SAVING_BAR_CLOSE:
				DrawBarButton(lpdis->itemState&ODS_SELECTED?12:11);
				break;
			}
		}

		return TRUE;
	case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED) {
				switch (LOWORD(wParam))
				{
				case IDC_STREAM_SAVING_BAR_GO:
					{
						bStreamSaving = !bStreamSaving;

						SendMessage(hwndDlg, WM_INITDIALOG, 0, 0);

						reset_menu();
					}

					break;
				case IDC_STREAM_SAVING_BAR_SKIP_TITLE:
					{
					}

					break;
				case IDC_STREAM_SAVING_BAR_SETPATH:
					{
						TCHAR buf[50];
						TCHAR szBuffer[MAX_PATH];

						LoadResString(hInstance, IDS_SAVING_FOLDER_TITLE, buf, sizeof(buf));
						if ( browse_folder(szBuffer, buf, hwndDlg) && lstrcmpi(strStreamSavingPath, szBuffer))
							lstrcpy((LPTSTR)(LPCTSTR)strStreamSavingPath, szBuffer);
					}

					break;
				case IDC_STREAM_SAVING_BAR_BROWSE:
					{
						if (PathFileExists(strStreamSavingPath)) {
							ShellExecute(NULL, "explore", strStreamSavingPath, NULL, NULL, SW_SHOW);
						}
					}

					break;
				case IDC_STREAM_SAVING_BAR_CLOSE:
					{
						DestroyWindow(hwndStreamSavingBar);
						hwndStreamSavingBar = NULL;

						reset_menu();
					}

					break; 
				}
			}
		}

		return TRUE;
	case WM_DESTROY:
		{
			if (himlStreamSavingBar) {
				ImageList_Destroy(himlStreamSavingBar); // delete image list
				himlStreamSavingBar = NULL;
			}

			if (hwndToolTip) {
				DestroyWindow(hwndToolTip);
				hwndToolTip = NULL;
			}

			RECT rc;
			GetWindowRect(hwndDlg, &rc);

			xStreamSavingBar = rc.left;
			yStreamSavingBar = rc.top;
		}

		return TRUE;
	}
	
	return FALSE; 
	UNREFERENCED_PARAMETER(lParam); 
}

