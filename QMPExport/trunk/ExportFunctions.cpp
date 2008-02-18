#include <time.h>

#include "xmlParser/xmlParser.h"

#include "QMPGeneralDLL.h"
#include "SongInfo.h"
#include "ExportFunctions.h"

//////////////////////////////////////////////////////////////////////

void ExportNowPlaying_Started()
{
	// Prepare data to be written
	CSongInfo songInfo;
	if (!songInfo.IsValid())
		return;

	WCHAR strTime[64] = {0};
	GetTimestamp(strTime, 64);

	// Build XML document
	XMLNode xMainNode, xNode;
	/*xMainNode = XMLNode::createXMLTopNode(L"xml", TRUE);
	xMainNode.addAttribute(L"version", L"1.0");
	xMainNode.addAttribute(L"encoding", L"utf-16");*/

	xMainNode = XMLNode::createXMLTopNode(L"now_playing", FALSE);
	xMainNode.addAttribute(L"playing", L"1");
	xMainNode.addAttribute(L"timestamp", strTime); // L"2004-05-24T23:12:46-07:00"

	xNode = xMainNode.addChild(L"song");
	
	xNode.addChild(L"title").addText(songInfo.GetTitle());
	xNode.addChild(L"artist").addText(songInfo.GetArtist());
	xNode.addChild(L"album").addText(songInfo.GetAlbum());
	xNode.addChild(L"genre").addText(songInfo.GetGenre());
	xNode.addChild(L"kind").addText(songInfo.GetFileFormat()); // <kind>MPEG audio file</kind>	
	xNode.addChild(L"track").addText(songInfo.GetTrackNumber());
	xNode.addChild(L"year").addText(songInfo.GetYear());
	xNode.addChild(L"comments").addText(songInfo.GetComment());
	xNode.addChild(L"time").addText(songInfo.GetLength());
	xNode.addChild(L"bitrate").addText(songInfo.GetBitrate());
   //   <playcount>2</playcount>
   //   <compilation>No</compilation>
   //   <urlAmazon>http://www.amazon.com/...</url>
   //   <urlApple>http://phobos.apple.com/...</url>
   //   <imageSmall>http://images.amazon.com/...jpg</imageSmall>
   //   <image>http://images.amazon.com/...jpg</image>
   //   <imageLarge>http://images.amazon.com/...jpg</imageLarge>
   xNode.addChild(L"composer").addText(songInfo.GetComposer());
   //   <grouping></grouping>
   xNode.addChild(L"file").addText(songInfo.GetFilePath());
   //   <urlSource></urlSource>
	
	// Extra fields
	xNode.addChild(L"rating").addText(songInfo.GetRating());

	XMLError err = xMainNode.writeToFile(Settings.strNowPlayingPath.GetUnicode());
	if (err == eXMLErrorCannotOpenWriteFile)
		MessageBox(0, L"Failed to open file", L"QMP Export", 0);

	songInfo.Destroy();
}

void ExportNowPlaying_Ended()
{
	WCHAR strTime[64] = {0};
	GetTimestamp(strTime, 64);

	// Build XML document
	XMLNode xMainNode, xNode;
	xMainNode = XMLNode::createXMLTopNode(L"xml", TRUE);
	xMainNode.addAttribute(L"version", L"1.0");
	xMainNode.addAttribute(L"encoding", L"utf-16");

	xNode = xMainNode.addChild(L"now_playing");
	xNode.addAttribute(L"playing", L"0");
	xNode.addAttribute(L"timestamp", strTime);

	XMLError err = xMainNode.writeToFile(Settings.strNowPlayingPath.GetUnicode());
}

void GetTimestamp(WCHAR* strTime, size_t nSize)
{
	WCHAR strTemp[32] = {0};
	long nTimezone = 0;
	time_t ltime = 0;
	tm today;

	_time64(&ltime); 
	_localtime64_s(&today, &ltime);
	wcsftime(strTemp, 32, L"%Y-%m-%dT%H:%M:%S", &today);

	if (_get_timezone(&nTimezone) == 0) // 0 == successful
		swprintf_s(strTime, nSize, L"%s%+0.2i:%0.2u", strTemp, nTimezone / -3600, nTimezone % 60);
}


//////////////////////////////////////////////////////////////////////
HANDLE hExportHtmlThread = 0;
ULONG nExportHtmlThreadId = 0;

void ExportHtml()
{
	if (hExportHtmlThread == 0)
	{
		hExportHtmlThread = CreateThread(NULL, 0, ExportHtmlThreadProc, g_hwndPlayer, 0, &nExportHtmlThreadId);
		if (hExportHtmlThread == NULL)
		{
			MessageBox(g_hwndPlayer, L"Failed to create export thread!", L"QMP Export", MB_ICONINFORMATION);
		}
	}
}

DWORD WINAPI ExportHtmlThreadProc(LPVOID lpParameter)
{
	HWND hwndPlayer = (HWND)lpParameter;

	// Check if done querying files
	if (!QMPCallbacks.Service(opGetQueriesComplete, NULL, 0, 0))
	{
		MessageBox(hwndPlayer, L"The player is not done querying information from all songs.\nPlease wait...", L"QMP Export", MB_ICONINFORMATION);

		while (!QMPCallbacks.Service(opGetQueriesComplete, NULL, 0, 0))
			Sleep(500);
	}

	long nPlaylistSize = 0;
	WCHAR wszFilePath[MAX_PATH] = {0};

	XMLNode xMainNode, xHtml, xHead, xBody, xNode, xTable;

	xMainNode = XMLNode::createXMLTopNode(L"xml", TRUE);
	xMainNode.addAttribute(L"version", L"1.0");
	xMainNode.addAttribute(L"encoding", L"utf-16");

	xMainNode.addClear(L" html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\"", L"<!DOCTYPE", L">", 0);

	xHtml = xMainNode.addChild(L"html");
	xHtml.addAttribute(L"xmlns", L"http://www.w3.org/1999/xhtml");
	xHtml.addAttribute(L"xml:lang", L"en");

	xHead = xHtml.addChild(L"head");
	xNode = xHead.addChild(L"title");
	xNode.addText(L"QMP Exported playlist");

	xNode = xHead.addChild(L"link");
	xNode.addAttribute(L"rel", L"stylesheet");
	xNode.addAttribute(L"type", L"text/css");
	xNode.addAttribute(L"href", L"style.css");

	xBody = xHtml.addChild(L"body");
	xTable = xBody.addChild(L"table");
	xTable.addAttribute(L"class", L"Playlist");
	
	nPlaylistSize = QMPCallbacks.Service(opGetNumTracks, NULL, 0, 0);
	for (int i = 0; i < nPlaylistSize; i++)
	{
		if (QMPCallbacks.Service(opGetPlaylistFile, wszFilePath, MAX_PATH*sizeof(WCHAR), i))
		{
			XMLNode xRow, xCol;
			CSongInfo songInfo(wszFilePath);
			if (!songInfo.IsValid())
				continue;

			xRow = xTable.addChild(L"tr");
			if (i % 2 == 0) xRow.addAttribute(L"class", L"Row_odd");
			else            xRow.addAttribute(L"class", L"Row_even");
			
			xCol = xRow.addChild(L"td");
			xCol.addAttribute(L"class", L"TrackNumber");
			xCol.addText(songInfo.GetTrackNumber());

			xCol = xRow.addChild(L"td");
			xCol.addAttribute(L"class", L"Title");
			xCol.addText(songInfo.GetTitle());

			xCol = xRow.addChild(L"td");
			xCol.addAttribute(L"class", L"Artist");
			xCol.addText(songInfo.GetArtist());
	
			xCol = xRow.addChild(L"td");
			xCol.addAttribute(L"class", L"Album");
			xCol.addText(songInfo.GetAlbum());

			xCol = xRow.addChild(L"td");
			xCol.addAttribute(L"class", L"Length");
			xCol.addText(songInfo.GetLength());

			xCol = xRow.addChild(L"td");
			xCol.addAttribute(L"class", L"FilePath");
			xCol.addText(songInfo.GetFilePath());
		}
	}

	
	XMLError err = xMainNode.writeToFile(Settings.strHtmlExportPath.GetUnicode());
	if (err == eXMLErrorNone) {
		if (IDYES == MessageBox(hwndPlayer, L"Done creating file.\n\nDo you want to open the file?", L"QMP Export", MB_YESNO))
			ShellExecute(NULL, L"open", Settings.strHtmlExportPath, 0, 0, SW_SHOWDEFAULT);
	}
	else if (err == eXMLErrorCannotOpenWriteFile)
		MessageBox(hwndPlayer, L"Failed to open file for writing.", L"QMP Export", MB_ICONERROR);
	else
		MessageBox(hwndPlayer, L"Failed export.", L"QMP Export", MB_ICONERROR);

	hExportHtmlThread = NULL;
	return 0;
}

//////////////////////////////////////////////////////////////////////
HANDLE hExportXmlThread = 0;
ULONG nExportXmlThreadId = 0;

void ExportXml()
{
	if (hExportXmlThread == 0)
	{
		hExportXmlThread = CreateThread(NULL, 0, ExportXmlThreadProc, g_hwndPlayer, 0, &nExportXmlThreadId);
		if (hExportXmlThread == NULL)
		{
			MessageBox(g_hwndPlayer, L"Failed to create export thread!", L"QMP Export", MB_ICONINFORMATION);
		}
	}
}

DWORD WINAPI ExportXmlThreadProc(LPVOID lpParameter)
{
	// http://xspf.org/
	// http://validator.xspf.org/	

	HWND hwndPlayer = (HWND)lpParameter;

	// Check if done querying files
	if (!QMPCallbacks.Service(opGetQueriesComplete, NULL, 0, 0))
	{
		MessageBox(hwndPlayer, L"The player is not done querying information from all songs.\nPlease wait...", L"QMP Export", MB_ICONINFORMATION);

		while (!QMPCallbacks.Service(opGetQueriesComplete, NULL, 0, 0))
			Sleep(500);
	}

	long nPlaylistSize = 0;
	WCHAR wszFilePath[MAX_PATH] = {0};

	XMLNode xMainNode, xTracklistNode, xTrackNode;

	xMainNode = XMLNode::createXMLTopNode(L"playlist", FALSE);
	xMainNode.addAttribute(L"version", L"1");
	xMainNode.addAttribute(L"xmlns", L"http://xspf.org/ns/0/");

	xTracklistNode = xMainNode.addChild(L"trackList");

	nPlaylistSize = QMPCallbacks.Service(opGetNumTracks, NULL, 0, 0);
	for (int i = 0; i < nPlaylistSize; i++)
	{
		if (QMPCallbacks.Service(opGetPlaylistFile, wszFilePath, MAX_PATH*sizeof(WCHAR), i))
		{
			CSongInfo songInfo(wszFilePath);

			xTrackNode = xTracklistNode.addChild(L"track");

			xTrackNode.addChild(L"location").addText(wszFilePath);
			if (!songInfo.IsValid())
				continue;

			xTrackNode.addChild(L"album").addText(songInfo.GetAlbum());
			xTrackNode.addChild(L"creator").addText(songInfo.GetArtist());
			xTrackNode.addChild(L"title").addText(songInfo.GetTitle());
			xTrackNode.addChild(L"trackNum").addText(songInfo.GetTrackNumber());
			xTrackNode.addChild(L"duration").addText(songInfo.GetLengthMs());
		}
	}

	
	XMLError err = xMainNode.writeToFile(Settings.strXmlExportPath.GetUnicode());
	if (err == eXMLErrorNone) {
		if (IDYES == MessageBox(hwndPlayer, L"Done creating file.\n\nDo you want to open the file?", L"QMP Export", MB_YESNO))
			ShellExecute(NULL, L"open", Settings.strXmlExportPath, 0, 0, SW_SHOWDEFAULT);
	}
	else if (err == eXMLErrorCannotOpenWriteFile)
		MessageBox(hwndPlayer, L"Failed to open file for writing.", L"QMP Export", MB_ICONERROR);
	else
		MessageBox(hwndPlayer, L"Failed export.", L"QMP Export", MB_ICONERROR);

	hExportXmlThread = NULL;
	return 0;
}

//////////////////////////////////////////////////////////////////////