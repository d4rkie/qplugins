#if !defined(QMPEXPORT_FUNCTIONS_H)
#define QMPEXPORT_FUNCTIONS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////

void ExportNowPlaying_Started();
void ExportNowPlaying_Ended();
void GetTimestamp(WCHAR* strTime, size_t nSize);

void ExportHtml();
DWORD WINAPI ExportHtmlThreadProc(LPVOID lpParameter);

void ExportXml();
DWORD WINAPI ExportXmlThreadProc(LPVOID lpParameter);

//////////////////////////////////////////////////////////////////////

#endif // QMPEXPORT_FUNCTIONS_H
