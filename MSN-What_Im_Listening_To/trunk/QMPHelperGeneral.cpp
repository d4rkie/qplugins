// #include <QCDModDefs.h>
#include "QCDGeneralDLL.h"
#include "QMPHelperGeneral.h"


// QCD 4.51 constants that isn't in QMP (b117+)
const static long LEGACY_DIGITAL_VIDEOFILE_MEDIA = 4;


PluginServiceFunc QMPService = NULL;

//-----------------------------------------------------------------------------
// IMPORTANT !!!
// Remember to run this before using any of the functions
//-----------------------------------------------------------------------------
void InitializeHelper(PluginServiceFunc Service)
{
	QMPService = Service;
}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

BOOL IsPlayerStatus(UINT status)
{
	BOOL bReturn = FALSE;

	const long nState = QMPService(opGetPlayerState, 0, 0, 0);
	switch (status)
	{
		case QMP_PLAYING :
			if (nState == 2)
				bReturn = TRUE;
			break;
		case QMP_STOPPED :
			if (nState == 1)
				bReturn = TRUE;
			break;
		case QMP_PAUSED :
			if (nState == 3)
				bReturn = TRUE;
			break;
	}

	return bReturn;
}

BOOL IsEncoding()
{
	// (1 = stopped, 2 = encoding, 0 = failed))
	const long nState = QMPService(opGetEncoderState, 0, 0, 0);
	if (nState == 2)
		return TRUE;
	return FALSE;
}

BOOL IsVideo()
{
	long nReturn = QMPService(opGetMediaType, 0, -1, 0);
	if ( (g_IsNewApi && nReturn == DIGITAL_VIDEO_MEDIA) ||
		  (!g_IsNewApi && nReturn == LEGACY_DIGITAL_VIDEOFILE_MEDIA) )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL IsStream()
{
	long nReturn = QMPService(opGetMediaType, 0, -1, 0);
	if (nReturn == DIGITAL_AUDIOSTREAM_MEDIA ||
	    nReturn == PLAYLIST_STREAM_MEDIA )
	{
		return TRUE;
	}
	
	return FALSE;
}