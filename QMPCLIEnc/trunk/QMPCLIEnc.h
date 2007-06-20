#pragma once

#include "QCDEncodeDLL.h"
#include "IQCDTagInfo.h"

#include "CLIEncoder.h"

#define PLUGIN_NAME L"CLI Encoder"

extern CString		g_strPath;
extern CString		g_strParameter;
extern CString		g_strExtension;
extern QCLIEncoder	g_cliEnc;
extern BOOL			g_bDoTag;
extern BOOL			g_bNoWAVHeader;
extern BOOL			g_bShowConsole;
extern CString		g_strEPFile;

