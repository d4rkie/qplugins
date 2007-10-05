#pragma once

#include <atlpath.h>

#include <map>
#include <vector>
using namespace std;

#include "QCDTagData.h"


//////////////////////////////////////////////////////////////////////////

class QCUESheet
{
public:
	QCUESheet();
	~QCUESheet();
private:
	typedef struct {
		CPath file;

		map< CString , CString > tags;
		int track;

		int start;
		int end;
	} cue_entry;

	map< unsigned int, cue_entry > m_mapTrackTable;
	FILE * _pf;

	int m_iVTNumber; //!< @var vt number when read from virtual track path.


public:
	CPath m_pathCUESheetFile; //!< CUE Sheet file path.


private:
	void _parse_command(const CString & inStr, CString & outCmd, vector< CString > & outPars);
	int  _get_frame_time(const CString & inStr);
	bool _read_string(FILE * fp, CString & strLine);


public:
	BOOL ReadFromCUEFile(const CPath & pathCUEFile);
	BOOL ReadFromVirtualTrack(const CPath & pathVirtualTrack);


public:

	UINT GetNumTracks() { return (UINT)m_mapTrackTable.size(); }

	CPath GetImageFilePath(UINT track) { return m_mapTrackTable[track].file; }

	map< CString, CString > & GetTrackTags(UINT track) { return m_mapTrackTable[track].tags; }
	CString GetTrackTagByName(UINT track, LPCTSTR name)
	{
		map< CString, CString > & tags = m_mapTrackTable[track].tags;
		map< CString, CString>::const_iterator cit = tags.find( name);
		return (cit != tags.end()) ? (cit->second) : _T("");
	}

	double GetTrackStartIndex(UINT track)
	{
		return m_mapTrackTable[track].start / 75.0;
	}
	INT GetTrackStartIndexFrame(UINT track) { return m_mapTrackTable[track].start; }
	double GetTrackEndIndex(UINT track, double total_end)
	{
		int start = m_mapTrackTable[track].start;
		int end = m_mapTrackTable[track].end;
		return ( ( (end < 0) || (end<=start) || ((end/75.0) > total_end) ) ? total_end : end/75.0 );
	}
	INT GetTrackEndIndexFrame(UINT track) { return m_mapTrackTable[track].end; }

	INT GetVirtualTrackNumber() { return m_iVTNumber; }
	CPath GetCUESheetFilePath() { return m_pathCUESheetFile; }

	static BOOL ParseVirtualTrackPath(const CPath & pathVirtualTrack, CPath * pathCUEFile = NULL, int * iVTNum = NULL);
};

