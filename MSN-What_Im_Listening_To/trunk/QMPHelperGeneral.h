#ifndef QMPHelperGeneral_H
#define QMPHelperGeneral_H 1

#include <QCDModDefs.h>


// Player status
static const UINT QMP_STOPPED = 1;
static const UINT QMP_PLAYING = 2;
static const UINT QMP_PAUSED  = 3;

void InitializeHelper(PluginServiceFunc Service);
BOOL IsPlayerStatus(UINT status);
BOOL IsEncoding();
BOOL IsVideo();
BOOL IsStream();

#endif // QMPHelperGeneral_H