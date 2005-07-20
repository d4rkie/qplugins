//==Preprocessors==
#ifndef QCDMIDIEXCEPTION_H
#define QCDMIDIEXCEPTION_H

// Note: It's been awhile, but I don't think I actually added this exception to
// the MIDI class.  This was to be added in a new version, but I never got around to it.

#include <windows.h>

//==QCD Midi Exception Class==
class QCDMidiException
{
public:
	//==Constructors==
	QCDMidiException(HWND window);
	QCDMidiException(HWND window, LPTCSTR error);
};

//==Overloaded Constructors==
QCDMidiException::QCDMidiException(HWND window)
{
	MessageBox(window, TEXT("An error has occurred and this plugin will exit."),
			   TEXT("QCD Midi Plugin"), MB_OK);
	exit(0);
}

QCDMidiException::QCDMidiException(HWND window, LPTCSTR error)
{
	MessageBox(window, error, TEXT("QCD Midi Plugin"), MB_OK);
	exit(0);
}

#endif
