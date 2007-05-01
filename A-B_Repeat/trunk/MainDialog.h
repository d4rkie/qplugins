#ifndef ABREPEAT_MAINDLG_H
#define ABREPEAT_MAINDLG_H

#include <QCDModDefs.h>

//#define IPC_GETOUTPUTTIME 105

void FormatTime(_TCHAR* str, size_t ccbSize, int iTimeIn);
void CreateMainDlg();

// Callbacks
BOOL CALLBACK cbMainDlg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // ABREPEAT_MAINDLG_H