#include "stdafx.h"

#include "QCUESheet.h"


//////////////////////////////////////////////////////////////////////////

QCUESheet::QCUESheet()
: _pf(NULL)
, m_iVTNumber(-1)
, m_pathCUESheetFile(_T(""))
{
	m_mapTrackTable.clear();
}

QCUESheet::~QCUESheet()
{
}

//////////////////////////////////////////////////////////////////////////

void QCUESheet::_parse_command(const CString & inStr, CString & outCmd, vector< CString > & outPars)
{
	CString str = inStr; str.Trim(); // trim leading & tailing whitespace

	// Get Command
	int t = str.Find( _T(" "));
	outCmd = (t >= 0) ? str.Left( t) : str;
	outCmd.MakeUpper(); // uppercase always

	// Get Parameters one by one
	while ( t >= 0) {
		// pop up one pattern
		str = str.Mid( t+1); str.TrimLeft();

		if ( str.IsEmpty())
			break;

		// find next token
		TCHAR c = (str[0] == _T('\"')) ? _T('\"') : _T(' ');
		t = str.Find( c, 1);

		CString par = (t >= 0) ? str.Left(t) : str;
		par.TrimLeft('\"'); // string parameters

		outPars.push_back( par);
	}
}

int QCUESheet::_get_frame_time(const CString & inStr)
{
	CString delimit = _T(":");
	int iStart = 0;

	// Get minute
	int value = _ttoi( inStr.Tokenize( delimit, iStart).GetBuffer());

	// Get second
	value = value * 60 + _ttoi( inStr.Tokenize( delimit, iStart).GetBuffer());

	// Get frame (75 frame per second)
	value = value * 75 + _ttoi( inStr.Tokenize( delimit, iStart).GetBuffer());

	return value;
}

bool QCUESheet::_read_string(FILE * fp, CString & strLine)
{
	const int BUFSIZE = 255;
	TCHAR buf[BUFSIZE]; ZeroMemory( buf, sizeof(TCHAR)*BUFSIZE);

	strLine.Empty();

	while ( _fgetts( buf, BUFSIZE, fp)) {
		strLine += buf;

		int end = BUFSIZE - 1 -1;
		if ( buf[end] == _T('\0') || buf[end] == _T('\n'))
			break;
		else
			ZeroMemory( buf, sizeof(CHAR)*255);
	}

	return !strLine.IsEmpty();
}

BOOL QCUESheet::ReadFromCUEFile(const CPath & pathCUESheetFile)
{
	if ( _tfopen_s( &_pf, pathCUESheetFile, _T("r, ccs=UTF-8")) != 0)
		return FALSE;

	// save the cue sheet file path
	m_pathCUESheetFile = pathCUESheetFile;

	CPath cur_file;
	int cur_track = -1;
	CString track_type;

	// tags types
	map< CString, CString > album_tags, track_tags;

	//CString rg_track_gain, rg_track_peak;
	//CString rg_album_gain, rg_album_peak;
	//CString track_title, track_performer;
	//CString album_title, album_performer;
	//CString genre;
	//CString year;

	cue_entry * entry = NULL;

	CString line;
	album_tags.clear(); track_tags.clear();
	while ( _read_string( _pf, line)) {
		line.Trim();

		CString cmd;
		vector< CString > pars;

		// Parse the string line and get the command and its parameters.
		_parse_command( line, cmd, pars);

		// Process the command
		if ( cmd == _T("FILE")) {
			// get & fix the path
			cur_file = CPath(pars[0]);
			if ( cur_file.IsRelative()) {
				cur_file = m_pathCUESheetFile;
				cur_file.RemoveFileSpec();
				cur_file.Append( pars[0]);
			}
		} else if ( cmd == _T("TRACK")) {
			cur_track = _ttoi( pars[0].GetBuffer());
			track_type = pars[1].MakeUpper();

			CString buf; buf.Format( _T("%d"), cur_track);
			track_tags[QCDTag_TrackNumber] = buf;
		} else if ( cmd == _T("INDEX")) {
			int index_no = _ttoi( pars[0].GetBuffer());
			int index_temp = _get_frame_time( pars[1].GetBuffer());
			if ( index_no == 1) {
				// push the created track
				if ( entry) {
					// set the end time if it is not set by index0
					if ( entry->end < 0) entry->end = index_temp;
					m_mapTrackTable[entry->track] = (*entry);
					delete entry; entry = NULL;
				}
				// we create a new cue entry on index1
				// only "AUDIO" track
				if ( track_type != _T("AUDIO")) continue;
				entry = new cue_entry;
				if ( entry) {
					entry->file = cur_file;
					// save tag fields
					{
						map< CString, CString > tags; // final tags
						map< CString, CString >::const_iterator cit;

						// add album tags
						tags = album_tags;
						// add track tags
						for ( cit = track_tags.begin(); cit != track_tags.end(); ++cit)
							tags[cit->first] = cit->second;

						// finish the final tags
						entry->tags = tags;
						
						// clear track tags, but keep the album tags
						track_tags.clear();
					}

					// we need save the track number
					entry->track = cur_track;

					entry->start = index_temp;
					entry->end = -1; // calculated by indx0 or determined on run-time
				}
			} else if ( index_no == 0) {
				// we got an index0 which mean a gap.
				if ( entry && (entry->end < 0) ) entry->end = index_temp;
			} else {
				; // Oops! ignore all other indexN
			}
		} else if ( cmd == _T("PERFORMER")) {
			if ( cur_track < 0)
				album_tags[QCDTag_AlbumArtist] = pars[0];
			else
				track_tags[QCDTag_AlbumArtist] = pars[0];
		} else if ( cmd == _T("TITLE")) {
			if ( cur_track < 0)
                album_tags[QCDTag_Album] = pars[0];
			else
				track_tags[QCDTag_Title] = pars[0];
		} else if ( cmd == _T("REM") && pars.size() > 1) {
			// parse tag
			//
			// NOTE: tag format: REM name value
			//   comment format: REM "bababaa..."
			CString name = pars[0];
			// fix tag key name
			if ( name.CompareNoCase( _T("DATE")) == 0)
				name = _T("YEAR");

			if ( cur_track < 0)
				album_tags[name] = pars[1];
			else
				track_tags[name] = pars[1];
		} else {
			continue;
		}
	}

	// push the last track into our track table
	if ( entry) {
		m_mapTrackTable[entry->track] = (*entry);
		delete entry; entry = NULL;
	}

	fclose( _pf);

	return m_mapTrackTable.size() > 0;
}

/************************************************************************
 ** Read the CUE Sheet file content from a virtual track path
 **
 ** The format of virtual track path:
 **   "....\foo.cue/1.vt"
 ***********************************************************************/
BOOL QCUESheet::ReadFromVirtualTrack(const CPath & pathVirtualTrack)
{
	CPath pathCUEFile;

	pathCUEFile = _T("");
	m_iVTNumber = -1;

	// First, parse the virtual track path
	if ( ! ParseVirtualTrackPath( pathVirtualTrack, &pathCUEFile, &m_iVTNumber))
		return FALSE;

	// Now, read the cue sheet content from cue sheet file path
	return ReadFromCUEFile( pathCUEFile);
}

/************************************************************************
 ** Parse a virtual track path
 **
 ** The format of virtual track path:
 **   "....\foo.cue/1.vt"
 ** Get the CUE Sheet file path and the virtual track number
 ***********************************************************************/
BOOL QCUESheet::ParseVirtualTrackPath(const CPath & pathVirtualTrack, CPath * pathCUEFile, int * iVTNum)
{
	int dotPos, tokenPos;

	// Find the position of the last "."
	dotPos = pathVirtualTrack.FindExtension();
	if ( dotPos < 0)
		return FALSE;

	// Find dot and check the ".vt" extension
	if ( pathVirtualTrack.m_strPath.Mid( dotPos).CompareNoCase( _T(".vt")))
		return FALSE;

	// Find token and check the ".cue" extension
	// The token can be any char.
	//   Default token is ","
	tokenPos = dotPos - 1;
	while ( (0 <= tokenPos) && (_istdigit( pathVirtualTrack.m_strPath[tokenPos]))) --tokenPos;
	if ( tokenPos < 0 || (1 >= dotPos - tokenPos)) // nothing
		return FALSE;
	if ( pathVirtualTrack.m_strPath.Mid( tokenPos - 4, 4).CompareNoCase( _T(".cue")))
		return FALSE;

	// Save the CUE Sheet file path
	if ( pathCUEFile) (*pathCUEFile) = (LPCTSTR)pathVirtualTrack.m_strPath.Left( tokenPos);

	// Save the virtual track number
	if ( iVTNum) *iVTNum = _ttoi( pathVirtualTrack.m_strPath.Mid( tokenPos + 1, dotPos - tokenPos));

	return TRUE;
}

