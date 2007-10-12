#include "stdafx.h"
#include "QMPCUE.h"
#include "QMPTags.h"

#include "IQCDTagInfo.h"

#include "QCUESheet.h"

#include "ConfigDlg.h"
#include "AboutDlg.h"

#include <set>
using namespace std;


//..............................................................................
// Global Variables
QCDModInitTag2	QMPTags::QCDCallbacks;

WCHAR g_szTagModDisplayStr[1024] = {0};

BOOL g_bNoteON = TRUE; //!< display the note dlg?

INT_PTR CALLBACK NoteDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL g_bOWIMGTags = TRUE;

//------------------------------------------------------------------------------

void _backup_cue_file(LPCTSTR cuepath)
{
	TCHAR cuebk[MAX_PATH+3];
	lstrcpy( cuebk, cuepath);
	lstrcat( cuebk, _T(".bk"));

	// show note dialog box
	if ( g_bNoteON)
		DialogBox( g_hInstance, MAKEINTRESOURCE( IDD_NOTE), GetForegroundWindow(), NoteDlgProc);

	// backup the original cue file
	CopyFile( cuepath, cuebk, TRUE);
}

//-----------------------------------------------------------------------------

BOOL QMPTags::Initialize(QCDModInfo *modInfo, int flags)
{
	TCHAR inifile[MAX_PATH];
	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, sizeof(TCHAR)*MAX_PATH, 0);

	g_bOWIMGTags = GetPrivateProfileInt( _T("CUE Sheet"), _T("OWIMGTags"), 1, inifile);

	modInfo->moduleString = (char *)g_szTagModDisplayStr;
	modInfo->moduleExtensions = (char *)L"VT";

	// load display name
	ResInfo resInfo = { sizeof(ResInfo), g_hInstance, MAKEINTRESOURCE(IDS_TAG_MODULE), 0, 0 };
	QCDCallbacks.Service(opLoadResString, (void*)g_szTagModDisplayStr, (long)sizeof(g_szTagModDisplayStr), (long)&resInfo);

	return TRUE;
}

//-----------------------------------------------------------------------------

void QMPTags::ShutDown(int flags)
{
	// Save settings
	TCHAR inifile[MAX_PATH];
	QCDCallbacks.Service( opGetPluginSettingsFile, inifile, sizeof(TCHAR)*MAX_PATH, 0);

	TCHAR buf[10];
	wsprintf( buf, _T("%d"), g_bOWIMGTags);
	WritePrivateProfileString( _T("CUE Sheet"), _T("OWIMGTags"), buf, inifile);
}

//-----------------------------------------------------------------------------

BOOL QMPTags::ReadFromFile(LPCWSTR filename, void* tagHandle, int flags)
{
	set< CString > tags_read; // virtual track tags have been read?

	CPath pathImageFile;
	int vtNum;
	QCUESheet cueSheet;

	if ( !cueSheet.ReadFromVirtualTrack( (LPCTSTR)filename))
		return FALSE;

	vtNum = cueSheet.GetVirtualTrackNumber();
	pathImageFile = cueSheet.GetImageFilePath(vtNum);

	int startIndex = 0;
	tags_read.clear();
	// If the tag info interface is available, Import tag infos from original image file.
	// Note: some fields should be overwrite with the cue sheet file info.
	IQCDTagInfo * piTag = (IQCDTagInfo *)QCDCallbacks.Service( opGetIQCDTagInfo, (char *)(LPCTSTR)pathImageFile, 0, 0);
	if ( piTag && piTag->ReadFromFile( flags)) {
		int index;
		DWORD wnl, bvl;
		QTAGDATA_TYPE type;

		index = 0;
		wnl = bvl = 0;
		while ( piTag->GetTagDataByIndex( index++, NULL, &wnl , &type, NULL, &bvl)) {
			// skip the ZERO length tag field
			if ( wnl <= 0 || bvl <= 0) continue;
			// get the information from the tag instance.
			++wnl;
			CStringW wn;
			LPBYTE bv = new BYTE[bvl]; ZeroMemory( bv, sizeof(BYTE)*bvl);
			// get the tag data.
			piTag->GetTagDataByIndex( index-1, wn.GetBuffer( wnl), &wnl, &type, bv, &bvl);
			wn.ReleaseBuffer();

			LPCWSTR pv = NULL;
			if ( g_bOWIMGTags && (pv = cueSheet.GetTrackTagByName( vtNum, wn)) && (0 != lstrlen( pv))) {
				// need overwrite and find the tag
				QCDCallbacks.toPlayer.SetTagData( tagHandle, wn, QTD_TYPE_STRINGUNICODE, (BYTE*)pv, sizeof(WCHAR)*(lstrlenW( pv)+1), &startIndex);
			} else {
				// pass the original value to virtual track.
				QCDCallbacks.toPlayer.SetTagData( tagHandle, wn, type, bv, bvl, &startIndex);
			}

			tags_read.insert( wn); // Marking the tag field which we have processed it.

			// clear all
			if (bv) delete [] bv;
		}
	}

	// release the tag info interface
	if ( piTag) piTag->Release();

	// overwrite the tag fields by cue info
	map< CString, CString > & tags = cueSheet.GetTrackTags( vtNum);
	map< CString, CString >::const_iterator cit;
	for ( cit = tags.begin(); cit != tags.end(); ++cit) {
		if ( tags_read.find( cit->first) != tags_read.end())
			continue; // skip the read tags
		else {
			// process the unread tags
			LPCWSTR pv = cit->second;
			QCDCallbacks.toPlayer.SetTagData( tagHandle, cit->first, QTD_TYPE_STRINGUNICODE, (BYTE*)pv, sizeof(WCHAR)*(lstrlenW( pv)+1), &startIndex);
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL QMPTags::WriteToFile(LPCWSTR filename, void* tagHandle, int flags)
{
	// TODO:
	// read metadata from tagHandle and set each field to supported tag
	//
	// Example:
	//
	// LPCWSTR szFieldName = L"Genre";
	// WCHAR szFieldValue[1024];
	// QCD_TAGDATA_TYPE type;
	// QCDCallbacks.toPlayer.GetTagDataByName(tagHandle, szFieldName, &type, (BYTE*)szFieldValue, sizeof(szFieldValue), NULL);
#if 0
	// write tag to file
	TCHAR src[MAX_PATH], dst[MAX_PATH];
	int track;
	QFile infile, outfile;
	QCD_TAGDATA_TYPE type;
	PBYTE pbData;
	DWORD dwDataLen;
	int startIndex;

	if ( !_parse_virtual_track( filename, src, track))
		return false;

	_backup_cue_file( src);

	lstrcpy( dst ,src);
	PathRemoveExtension( dst);
	PathAddExtension( dst, _T(".tmp"));

	if ( !infile.Open( src, QFile::modeRead | QFile::shareExclusive) ||
        !outfile.Open( dst, QFile::modeCreate | QFile::modeWrite | QFile::shareExclusive))
		return false;

	CString line, tmpline;
	bool got = false, in_tracks = false;
	while ( infile.ReadString( line)) {
		tmpline = line;
		tmpline.TrimLeft( _T(" "));
		if ( tmpline.Left( 10) == _T("PERFORMER ")) {
			if ( !in_tracks) { // for album artist
				line = _T("PERFORMER \"");
				if ( QCDTagCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_AlbumArtist, &type, NULL, &dwDataLen, &startIndex) && type == QTD_TYPE_STRINGUNICODE) {
					pbData = new BYTE[++dwDataLen];
					QCDTagCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_AlbumArtist, &type, pbData, &dwDataLen, &startIndex);
					line += (LPWSTR)pbData;
					delete [] pbData;
				} else {
					line += _T("Unknow Artist");
				}
				line += _T("\"");
			} else { // for track artist
				if ( in_tracks && got) {
					line = _T("    PERFORMER \"");
					if ( QCDTagCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_Artist, &type, NULL, &dwDataLen, &startIndex) && type == QTD_TYPE_STRINGUNICODE) {
						pbData = new BYTE[++dwDataLen];
						QCDTagCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_Artist, &type, pbData, &dwDataLen, &startIndex);
						line += (LPWSTR)pbData;
						delete [] pbData;
					} else {
						line += _T("Unknow Artist");
					}
					line += _T("\"");
				}
			}
		} else if ( tmpline.Left( 6) == _T("TITLE ")) {
			if ( !in_tracks) { // for album title
				line = _T("TITLE \"");
				if ( QCDTagCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_Album, &type, NULL, &dwDataLen, &startIndex) && type == QTD_TYPE_STRINGUNICODE) {
					pbData = new BYTE[++dwDataLen];
					QCDTagCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_Album, &type, pbData, &dwDataLen, &startIndex);
					line += (LPWSTR)pbData;
					delete [] pbData;
				} else {
					line += _T("Unknow Title");
				}
				line += _T("\"");
			} else { // for track title
				if ( got) {
					line = _T("    TITLE \"");
					if ( QCDTagCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_Title, &type, NULL, &dwDataLen, &startIndex) && type == QTD_TYPE_STRINGUNICODE) {
						pbData = new BYTE[++dwDataLen];
						QCDTagCallbacks.toPlayer.GetTagDataByName( tagHandle, QCDTag_Title, &type, pbData, &dwDataLen, &startIndex);
						line += (LPWSTR)pbData;
						delete [] pbData;
					} else {
						line += _T("Unknow Title");
					}
					line += _T("\"");
				}
			}
		} else if ( tmpline.Left( 6) == _T("TRACK ")) {
			in_tracks = true;

			TCHAR * p = tmpline.GetBuffer( 0) + 6;
			while ( *p == _T(' ')) p++;
			got = (_ttoi( p) == track);
		}

		outfile.WriteString( line);
		outfile.WriteString( _T("\n"));
	}

	outfile.Close();
	infile.Close();

	DeleteFile( src);
	MoveFile( dst, src);
#endif
	// return true for successful writing, false for failure
	return true;
}

//-----------------------------------------------------------------------------

BOOL QMPTags::StripFromFile(LPCWSTR filename, void* tagHandle, int flags)
{
	// TODO:
	// remove tag from file.
	// do whatever is need to remove the supported tag from filename

	// return true for successful stripping, false for failure
	return TRUE;
}

//-----------------------------------------------------------------------------

void QMPTags::About(int flags)
{
	CAboutDlgTags dlg;
	dlg.DoModal( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0));
}

//-----------------------------------------------------------------------------

void QMPTags::Configure(int flags)
{
	CConfigDlgTags dlg;
	dlg.DoModal( (HWND)QCDCallbacks.Service( opGetPropertiesWnd, NULL, 0, 0));
}

//-----------------------------------------------------------------------------

INT_PTR CALLBACK NoteDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg)
	{
	case WM_INITDIALOG:
		CheckDlgButton( hwndDlg, IDC_NOTE_ON, g_bNoteON);

		return TRUE;
	case WM_CLOSE:
	case WM_COMMAND:
		switch ( LOWORD(wParam))
		{
		case IDC_NOTE_ON:
			g_bNoteON = !g_bNoteON;

			return TRUE;
		case IDOK:
		case IDCLOSE:
			EndDialog( hwndDlg, 0);
		}
	}
	return FALSE;
}

