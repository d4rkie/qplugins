#pragma once

#include "QCDEncodeDLL.h"

#include "CLIEncoder.h"

#define PLUGIN_NAME "CLI Encoder"
#define PLUGIN_FULL_NAME "Commandline Encoder"
#define PLUGIN_VERSION "1.62"

extern CString		g_strPath;
extern CString		g_strParameter;
extern CString		g_strExtension;
extern QCLIEncoder	g_cliEnc;
extern BOOL			g_bDoTag;
extern BOOL			g_bNoWAVHeader;
extern BOOL			g_bShowConsole;
extern TCHAR		g_szEPFile[MAX_PATH];

