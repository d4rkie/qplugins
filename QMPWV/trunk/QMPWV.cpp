#include "stdafx.h"
#include "QMPWV.h"

#include "QMPInput.h"
#include "QMPFileInfo.h"

//////////////////////////////////////////////////////////////////////////
// Global variables
static HINSTANCE	hInstance;    // The unique module instance handle

INT g_bUseWVC;

//-----------------------------------------------------------------------------
// The unique DLL main function

int WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if ( fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls( hInst);

		hInstance = hInst;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// Export Input Module

PLUGIN_API QCDModInitIn* INPUTDLL_ENTRY_POINT()
{
	return QMPInput::GetModule()->ExportInterface();
}

//------------------------------------------------------------------------------
// Export FileInfo Module

PLUGIN_API QCDModInitFileInfo* FILEINFO_DLL_ENTRY_POINT()
{
	return QMPFileInfo::GetModule()->ExportInterface();
}

