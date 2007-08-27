#include "stdafx.h"
#include "QMPTAK.h"

#include "QMPInput.h"
#include "QMPFileInfo.h"


//////////////////////////////////////////////////////////////////////////
// Global variables
HINSTANCE g_hInstance; // The unique module instance handle

//-----------------------------------------------------------------------------
// The unique DLL entry point

int WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if ( fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls( hInst);

		g_hInstance = hInst;
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

