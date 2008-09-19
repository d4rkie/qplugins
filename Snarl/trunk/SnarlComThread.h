#ifndef SnarlComThread_H
#define SnarlComThread_H

#pragma once


#include "QCDGeneralDLL.h"


bool RegisterWithSnarl();
void DisplaySongInfo();
bool IsPlaying();


static const int WM_REGISTER_MSG = 0; // 62091;

static LPCSTR QMP_APP_NAME        = "Quintessential Media Player";
static LPCSTR DEFAULT_SNARL_CLASS = "Track changes";



DWORD WINAPI SnarlCom_Main(LPVOID lpParameter)
{
	BOOL bRet;
	MSG  msg;
	MSG  msgPeak;

	PeekMessage(&msg, NULL, 0, 0xFFFF, PM_NOREMOVE); // force message queue to be created
	Sleep(0);

	if (!RegisterWithSnarl()) // Creates snarlInterface object
		return FALSE;

	//snarlInterface->snShowMessageEx("", "snShowMessageEx", "Test", 10, "", 0, 0, "");


	//////////////////////////////////////////////////////////////////////////
	// Thread message loop
	//////////////////////////////////////////////////////////////////////////
	OutputDebugInfo(L"Entering Snarl thread message loop");

	while( (bRet = GetMessage(&msg, NULL, 0, 0)) != 0 )
	{
		if (bRet == -1)
		{
			// Handle the error
			break;
		}
		else
		{
			// We only want to handle the last message posted, so check if msg queue is empty
			if (PeekMessage(&msgPeak, NULL, 0, 0, PM_NOREMOVE) == 0)
			{
				switch (msg.message)
				{
				case WM_PN_TRACKCHANGED :
					OutputDebugInfo(L"Thread received: WM_PN_TRACKCHANGED");
					Sleep(100);
					DisplaySongInfo();
					break;

				case WM_PN_PLAYSTARTED :
					OutputDebugInfo(L"Thread received: WM_PN_PLAYSTARTED");
					DisplaySongInfo();
					break;

				default :
					if ( (msg.message == g_nSnarlGlobalMessage) && (snarlInterface != NULL) && (msg.wParam == SnarlInterface::SNARL_LAUNCHED) )
					{
						OutputDebugInfo(L"Thread received: SNARL_LAUNCHED");
						RegisterWithSnarl();
					}

					break;
				}
			}
		}

	} // while

	//////////////////////////////////////////////////////////////////////////
	// Shutdown thread
	//////////////////////////////////////////////////////////////////////////
	if (Settings.bCloseSnarl)
		CloseSnarl();
	else
		snarlInterface->RevokeConfig(hwndPlayer);
	
	OutputDebugInfo(L"Snarl thread ended");
	return 0;
}


//-----------------------------------------------------------------------------

bool RegisterWithSnarl()
{
	SnarlInterface::M_RESULT snResult = SnarlInterface::M_FAILED;
	
	snResult = snarlInterface->RegisterConfig2(hwndPlayer, QMP_APP_NAME, WM_REGISTER_MSG, g_strDefaultIcon.GetUTF8());
	if (snResult != SnarlInterface::M_OK)
		OutputDebugInfo(L"Failed to register Snarl config");

	snResult = snarlInterface->RegisterAlert(QMP_APP_NAME, DEFAULT_SNARL_CLASS);
	if (snResult != SnarlInterface::M_OK)
		OutputDebugInfo(L"snRegisterAlert failed with error code: %x", (unsigned int)snResult);

	g_nSnarlGlobalMessage = snarlInterface->GetGlobalMsg();
	g_nSnarlVersion = snarlInterface->GetVersionEx();

	if (g_nSnarlVersion < 38)
		snarlInterface->ShowMessage("Please upgrade Snarl", "You are using an outdated version of Snarl. Please upgrade!");
	
	if (IsPlaying())
		PostThreadMessage(GetCurrentThreadId(), WM_PN_PLAYSTARTED, 0, 0);

	return TRUE;
}

//-----------------------------------------------------------------------------

void InsertNextLine(std::wstring* str, size_t nPos)
{
	if (nPos >= str->length())
		return;

	int nSpace = str->rfind(L" ", nPos);
	if (nSpace > 0)
		str->replace(nSpace, 1, L"\n");
}

//-----------------------------------------------------------------------------

void DisplaySongInfo()
{
	const static int nHeadlineChars = 26;
	const static int nText1Chars = 30;
	const static int nText2Chars = 30;

	if (!snarlInterface)
		return;

	WCHAR strTmp[SnarlInterface::SNARL_STRING_LENGTH];
	std::wstring strObj;
	QString strHeadline;
	QString strText1;
	QString strText2;
	QString strIcon;

	long nTrack = Service(opGetCurrentIndex, NULL, 0, 0);

	/*while ( !Service(opGetQueriesComplete, NULL, 0, 0) ){
		OutputDebugInfo(L"opGetQueriesComplete2 failed");
		Sleep(100);
	}
	OutputDebugInfo(L"opGetQueriesComplete2 done");*/

	// Retrieve song information from QMP
	Service(Settings.Headline_ServiceOp, strTmp, SnarlInterface::SNARL_STRING_LENGTH, nTrack);
	strHeadline = strTmp;
	Service(Settings.Text1_ServiceOp,    strTmp, SnarlInterface::SNARL_STRING_LENGTH, nTrack);
	strText1 = strTmp;
	Service(Settings.Text2_ServiceOp,    strTmp, SnarlInterface::SNARL_STRING_LENGTH, nTrack);
	strText2 = strTmp;

	strObj = strHeadline;
	if (Settings.bHeadline_wrap && strHeadline.Length() > nHeadlineChars)
		InsertNextLine(&strObj, nHeadlineChars);
	strHeadline.SetUnicode(strObj.c_str());

	if (Settings.Text2_ServiceOp != 0) {
		strText1.AppendUnicode(L"\n");
		strText1.AppendUnicode(strText2);
	}

	if (Settings.bDisplayCoverArt)
		GetCoverArt(nTrack, &strIcon);

	// Hide if not cascade
	if (!Settings.bCascade && snarlInterface->IsMessageVisible())
	{
		if (SnarlInterface::M_OK != snarlInterface->UpdateMessage(strHeadline.GetUTF8(), strText1.GetUTF8(), (strIcon.Length() > 0) ? strIcon.GetUTF8() : g_strDefaultIcon.GetUTF8()))
			snarlInterface->ShowMessageEx(DEFAULT_SNARL_CLASS, strHeadline.GetUTF8(), strText1.GetUTF8(), Settings.nTimeout, (strIcon.Length() > 0) ? strIcon.GetUTF8() : g_strDefaultIcon.GetUTF8(), 0, 0, "");
	}
	else
		snarlInterface->ShowMessageEx(DEFAULT_SNARL_CLASS, strHeadline.GetUTF8(), strText1.GetUTF8(), Settings.nTimeout, (strIcon.Length() > 0) ? strIcon.GetUTF8() : g_strDefaultIcon.GetUTF8(), 0, 0, "");
}

//-----------------------------------------------------------------------------

bool IsPlaying()
{
	return (Service(opGetPlayerState, 0, 0, 0) == 2);
}

//-----------------------------------------------------------------------------

#endif // SnarlComThread_H