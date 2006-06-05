// CfgFile.cpp: implementation of the CCfgFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WinLIRC.h"
#include "CfgFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCfgFile::CCfgFile(CString strFile)
{
	m_strFile = strFile;
}

CCfgFile::~CCfgFile()
{

}

//////////////////////////////////////////////////////////////////////
// Load function
//////////////////////////////////////////////////////////////////////
VOID CCfgFile::Read()
{
	const int BUFFER_SIZE = 1024;

	INT iRead = 0, iFind = 0, iFindEOL;
	_TCHAR strBuff[BUFFER_SIZE];
	CString strRead, strTemp;
	Command* pCmd;

	try {
		CFile oFile(m_strFile, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeRead | CFile::shareExclusive);

		CFileStatus status;
		if (oFile.GetStatus(status)) {
			if (status.m_size < 10) { // Lets just be safe... cfg can never be that small
				LoadDefault();
				return;			// No need to read rest of file that doesn't exist anyway ;-)
			}
		}

		// Read the whole file into strRead CString object
		iRead = oFile.Read(strBuff, BUFFER_SIZE - 1);
		strBuff[iRead] = 0;
		while (iRead == BUFFER_SIZE - 1)
		{
			strRead += strBuff;
			iRead = oFile.Read(strBuff, BUFFER_SIZE - 1);
			strBuff[iRead] = 0;
		}
		strRead += strBuff;
	}
	catch (CFileException e) {
		CString strErr;
		strErr.Format("Error when reading config file! Error return:\n%s", e);
		AfxMessageBox(strErr);
	}

	// Find each :NewButton and create new Command
	if (!strRead.IsEmpty()) {
		iFind = strRead.Find(_T(":NewButton"), 0);

		while (iFind > -1)
		{
			pCmd = new Command;

			// Find strCommand
			iFind = strRead.Find(_T("\n"), iFind + 1);
			iFindEOL = strRead.Find(_T("\n"), iFind + 1);
			pCmd->strCommand = strRead.Mid(iFind + 1, iFindEOL - iFind - 2);

			// Find strButton
			iFind = strRead.Find(_T("\n"), iFind + 1);
			iFindEOL = strRead.Find(_T("\n"), iFind + 1);
			pCmd->strButton = strRead.Mid(iFind + 1, iFindEOL - iFind - 2);

			// Find iCommand
			iFind = strRead.Find(_T("\n"), iFind + 1);
			iFindEOL = strRead.Find(_T("\n"), iFind + 1);
			strTemp = strRead.Mid(iFind + 1, iFindEOL - iFind - 2);
			pCmd->iCommand = atoi(strTemp);

			// Find bSendOnce
			iFind = strRead.Find(_T("\n"), iFind + 1);
			iFindEOL = strRead.Find(_T("\n"), iFind + 1);
			strTemp = strRead.Mid(iFind + 1, iFindEOL - iFind - 2);
			if (strTemp.CompareNoCase(_T("true")) == 0)
				pCmd->bSendOnce = TRUE;
			else
				pCmd->bSendOnce = FALSE;

			// Find iRepeatCount
			iFind = strRead.Find(_T("\n"), iFind + 1);
			iFindEOL = strRead.Find(_T("\n"), iFind + 1);
			strTemp = strRead.Mid(iFind + 1, iFindEOL - iFind - 2);
			pCmd->iRepeatCount = atoi(strTemp);

			// Find bCustom
			iFind = strRead.Find(_T("\n"), iFind + 1);
			iFindEOL = strRead.Find(_T("\n"), iFind + 1);
			strTemp = strRead.Mid(iFind + 1, iFindEOL - iFind - 2);
			if (strTemp.CompareNoCase(_T("true")) == 0)
				pCmd->bCustom = TRUE;
			else
				pCmd->bCustom = FALSE;

			pArrCommands->Add(pCmd);
			
			iFind = strRead.Find(_T(":NewButton"), iFindEOL);
		}

		/*for (int i = 0; i < pArrCommands->GetSize(); i++)
			MessageBox(hwndPlayer, ((Command*)pArrCommands->GetAt(i))->strButton, "Test", MB_OK);*/
	}
}

//////////////////////////////////////////////////////////////////////
// Save function
//////////////////////////////////////////////////////////////////////
VOID CCfgFile::Save()
{
	// This could be optimized easy by a fast string class... Hopefully the compiler does it though
	CString strWrite = _T(""), strTemp = _T("");
	_TCHAR strInt[16];

	// Header of the cfg file
	strWrite = _T(":Internal button name\r\n:WinLIRC button id\r\n:QCD_COMMAND\r\n"
		":Only on first message\r\n:Times to send message\r\n:Custom\r\n\r\n");

	// All the commands
	for (int i = 0; i < pArrCommands->GetSize(); i++)
	{
		strTemp = _T(":NewButton\r\n");

		strTemp += ((Command*)pArrCommands->GetAt(i))->strCommand + _T("\r\n");
		strTemp += ((Command*)pArrCommands->GetAt(i))->strButton + _T("\r\n");

		itoa(((Command*)pArrCommands->GetAt(i))->iCommand, strInt, 10);
		strTemp += strInt;
		strTemp += _T("\r\n");

		if (((Command*)pArrCommands->GetAt(i))->bSendOnce)
			strTemp += _T("true");
		else
			strTemp += _T("false");
		strTemp += _T("\r\n");

		itoa(((Command*)pArrCommands->GetAt(i))->iRepeatCount, strInt, 10);
		strTemp += strInt;
		strTemp += _T("\r\n");

		if (((Command*)pArrCommands->GetAt(i))->bCustom)
			strTemp += _T("true");
		else
			strTemp += _T("false");
		strTemp += _T("\r\n");


		// Add a line between buttons
		strTemp += _T("\r\n");
		strWrite += strTemp;
	}

	try {
		CFile oFile(m_strFile, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);
		oFile.Write(strWrite, strWrite.GetLength());
		oFile.Flush();
	}
	catch (CFileException e) {
		AfxMessageBox("Error:");
	}
}

//////////////////////////////////////////////////////////////////////
// Load Defaults
//////////////////////////////////////////////////////////////////////
VOID CCfgFile::LoadDefault()
{
	Command* pCmd;

	// Insert all elements
	pCmd = new Command;
	pCmd->bCustom		= false;
	pCmd->bSendOnce		= true;
	pCmd->iCommand		= 40016;
	pCmd->iRepeatCount	= 1;
	pCmd->strButton		= _T("play");
	pCmd->strCommand	= _T("Play");
	pArrCommands->Add(pCmd);

	pCmd = new Command;
	pCmd->bCustom		= false;
	pCmd->bSendOnce		= true;
	pCmd->iCommand		= 40014;
	pCmd->iRepeatCount	= 1;
	pCmd->strButton		= _T("stop");
	pCmd->strCommand	= _T("Stop");
	pArrCommands->Add(pCmd);

	pCmd = new Command;
	pCmd->bCustom		= false;
	pCmd->bSendOnce		= true;
	pCmd->iCommand		= 40015;
	pCmd->iRepeatCount	= 1;
	pCmd->strButton		= _T("pause");
	pCmd->strCommand	= _T("Pause");
	pArrCommands->Add(pCmd);

	pCmd = new Command;
	pCmd->bCustom		= false;
	pCmd->bSendOnce		= true;
	pCmd->iCommand		= 40013;
	pCmd->iRepeatCount	= 1;
	pCmd->strButton		= _T("Next Track");
	pCmd->strCommand	= _T("next");
	pArrCommands->Add(pCmd);

	pCmd = new Command;
	pCmd->bCustom		= false;
	pCmd->bSendOnce		= true;
	pCmd->iCommand		= 40016;
	pCmd->iRepeatCount	= 1;
	pCmd->strButton		= _T("prev");
	pCmd->strCommand	= _T("Previous Track");
	pArrCommands->Add(pCmd);

	pCmd = new Command;
	pCmd->bCustom		= false;
	pCmd->bSendOnce		= false;
	pCmd->iCommand		= 0;
	pCmd->iRepeatCount	= 1;
	pCmd->strButton		= _T("vol+");
	pCmd->strCommand	= _T("Volume Up (+1)");
	pArrCommands->Add(pCmd);

	pCmd = new Command;
	pCmd->bCustom		= false;
	pCmd->bSendOnce		= false;
	pCmd->iCommand		= 41027;
	pCmd->iRepeatCount	= 1;
	pCmd->strButton		= _T("vol-");
	pCmd->strCommand	= _T("Volume Down (-1)");
	pArrCommands->Add(pCmd);

	pCmd = new Command;
	pCmd->bCustom		= false;
	pCmd->bSendOnce		= true;
	pCmd->iCommand		= 0;
	pCmd->iRepeatCount	= 1;
	pCmd->strButton		= _T("mute");
	pCmd->strCommand	= _T("Mute");
	pArrCommands->Add(pCmd);
}