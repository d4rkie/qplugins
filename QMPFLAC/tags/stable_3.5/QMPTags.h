#ifndef QMPTags_H
#define QMPTags_H

#include "QCDModTagEditor2.h"

#include "QMPModule.h"


//////////////////////////////////////////////////////////////////////////

class QMPTags : public QMPModule< QMPTags, QCDModInitTag2 >
{
protected:
	friend class QMPModule< QMPTags, QCDModInitTag2 >;
	QMPTags(void)
	{
		QCDCallbacks.version				= PLUGIN_API_VERSION_UNICODE;

		QCDCallbacks.toModule.Initialize	= Initialize;
		QCDCallbacks.toModule.ShutDown		= ShutDown;
		QCDCallbacks.toModule.About			= About;
		QCDCallbacks.toModule.Configure		= NULL;//Configure;
		QCDCallbacks.toModule.ReadFromFile	= ReadFromFile;
		QCDCallbacks.toModule.WriteToFile	= WriteToFile;
		QCDCallbacks.toModule.StripFromFile	= StripFromFile;
	}

private:
	static BOOL Initialize(QCDModInfo *modInfo, int flags);
	static void ShutDown(int flags);

	static BOOL ReadFromFile(LPCWSTR filename, void* tagHandle, int flags);
	static BOOL WriteToFile(LPCWSTR filename, void* tagHandle, int flags);
	static BOOL StripFromFile(LPCWSTR filename, void* tagHandle, int flags);

	static void About(int flags);
	static void Configure(int flags);
};

#endif //QMPTags_H

