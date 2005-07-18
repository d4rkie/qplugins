#pragma once

#include "QCDEncodeDLL.h"

#include "CLIEncoderPreset.h"
#include "CLIEncoder.h"
#include "CLIEncoderUI.h"


extern QCLIEncoderPreset	g_cliEP;
extern QCLIEncoder			g_cliEnc;
extern ENCODER_PRESET		g_epCur;
extern BOOL					g_bNoWAVHeader;
extern BOOL					g_bShowConsole;
extern TCHAR				g_szEPFile[MAX_PATH];

