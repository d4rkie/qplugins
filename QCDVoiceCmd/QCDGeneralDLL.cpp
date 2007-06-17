//-----------------------------------------------------------------------------
//
// File:	QCDGeneralDLL.cpp
//
// About:	See QCDGeneralDLL.h
//
// Authors:	Written by Paul Quinn and Richard Carlson.
//
//	QCD multimedia player application Software Development Kit Release 1.0.
//
//	Copyright (C) 1997-2002 Quinnware
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

// QCDVoiceCmd.dll
// Created by Anthony Cozzolino
// Copyright 2002

//==Preprocessors==
#include "QCDGeneralDLL.h"
#include <sphelper.h>
#include <string>
#include "resource.h"
#include "QCDVoiceCmd.h"
using std::string;

//==Structures==
struct ThreadFlags
{
	bool init;
	bool shutdown;
	bool processreco;
};

//==Prototypes==
LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void ProcessRecoEvent(HWND hwnd);
void ExecuteCommand(ISpPhrase *phrase, HWND hwnd);
DWORD WINAPI ThreadProc(/*void* threadflags*/);

//==Global Variables==
CComPtr<ISpRecognizer> Engine;
CComPtr<ISpRecoContext> RecoCtxt;
CComPtr<ISpRecoGrammar> CmdGrammar;
HWND hVoice;
string classname = "QCDVoiceCmd";
const int WM_RECOEVENT = WM_APP;
const int WM_PLAY = WM_APP + 1;
const int WM_STOP = WM_APP + 2;
const int WM_PAUSE = WM_APP + 3;
const int WM_ADVANCE = WM_APP + 4;
const int WM_PREVIOUS = WM_APP + 5;

//==QCD Global Variables==
HINSTANCE		hInstance;
HWND			hwndPlayer;
QCDModInitGen	*QCDCallbacks;

//-----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID pRes)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		hInstance = hInst;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------

PLUGIN_API BOOL GENERALDLL_ENTRY_POINT(QCDModInitGen *ModInit, QCDModInfo *ModInfo)
{
	ModInit->version = PLUGIN_API_VERSION;
	ModInfo->moduleString = "Voice Command plugin v1.0";

	ModInit->toModule.ShutDown			= ShutDown;
	ModInit->toModule.About				= About;
	ModInit->toModule.Configure			= Configure;

	QCDCallbacks = ModInit;

	hwndPlayer = (HWND)QCDCallbacks->Service(opGetParentWnd, 0, 0, 0);

	char inifile[MAX_PATH];
	QCDCallbacks->Service(opGetPluginSettingsFile, inifile, MAX_PATH, 0);

	//
	// TODO: all your plugin initialization here
	//

	MSG msg;

	WNDCLASSEX wcx = {0};	// Window Class (Extended) Object

	//myInstance = hInstance;

	// Fill in class fields
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = WinProc;
	wcx.hInstance = hInstance;
	wcx.lpszClassName = classname.c_str();

	// Register class
	RegisterClassEx(&wcx);
	
	

	// Create Window (Extended)
	hVoice = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_CLIENTEDGE,
						  classname.c_str(),
						  "QCDVoiceCmd",
						  WS_MINIMIZEBOX | WS_SYSMENU | WS_OVERLAPPED | WS_CAPTION,
						  CW_USEDEFAULT,
						  CW_USEDEFAULT,
						  0,
						  0,
						  hwndPlayer,
						  NULL,
						  hInstance,
						  NULL);

	if(!hVoice)
	{
		return FALSE;
	}

	// Update and Show Window
	//UpdateWindow(myHandle);
	//ShowWindow(myHandle, SW_SHOW);

	ThreadFlags tf;
	tf.init = true;
	tf.shutdown = false;
	tf.processreco = false;

	
	

	
		
	if (PeekMessage(&msg, hVoice, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	//
	// TODO : Cleanup
	//
	CoUninitialize();
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	//
	// TODO : Show your "configuration" dialog.
	//
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	//
	// TODO : Show your "about" dialog.
	//
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
	/*case WM_CREATE:
		{
			MessageBox(hwndPlayer, "CoInit NULL", "debug", MB_OK);
			if (FAILED(CoInitialize(NULL)))
			{
				DestroyWindow(hwnd);
				return FALSE;
			}
			MessageBox(hwndPlayer, "CoInit SharedRec", "debug", MB_OK);
		HRESULT hr = Engine.CoCreateInstance(CLSID_SpSharedRecognizer);
		if (FAILED(hr))
		{
			DestroyWindow(hwnd);
			CoUninitialize();
			return FALSE;
		}
		MessageBox(hwndPlayer, "CoInit RecoCtxt", "debug", MB_OK);
		// create the command recognition context
		hr = Engine->CreateRecoContext(&RecoCtxt );
		if (FAILED(hr))
		{
			DestroyWindow(hwnd);
			CoUninitialize();
			return FALSE;
		}
	
		hr = RecoCtxt->SetNotifyWindowMessage(hwnd, WM_RECOEVENT, 0, 0);
		if (FAILED(hr))
		{
			DestroyWindow(hwnd);
			CoUninitialize();
			return FALSE;
		}
		
		hr = RecoCtxt->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
			if (FAILED(hr)) 
			{
				DestroyWindow(hwnd);
				CoUninitialize();
				return FALSE;
			}

		// Load our grammar
		// user defined
		//("GRAMMAR") resource type.
		MessageBox(hwndPlayer, "CoInit CreateGramm", "debug", MB_OK);
		hr = RecoCtxt->CreateGrammar(0, &CmdGrammar);
		if (FAILED(hr))
		{
			DestroyWindow(hwnd);
			CoUninitialize();
			return FALSE;
		}

		MessageBox(hwndPlayer, "Load Cmd", "debug", MB_OK);
		hr = CmdGrammar->LoadCmdFromResource(
			NULL,
			MAKEINTRESOURCEW(IDR_QCDGRAMMAR),
			L"SRGRAMMAR",
			MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL), SPLO_STATIC);

		if ( FAILED( hr ) ) //Leave application
		{
			DestroyWindow(hwnd);
			CoUninitialize();
			return FALSE;
		}

		hr = CmdGrammar->SetRuleState( NULL, NULL, SPRS_ACTIVE );
		if (FAILED(hr))
		{
			DestroyWindow(hwnd);
			CoUninitialize();
			return FALSE;
		}

		return 0;
		}*/

	case WM_DESTROY:case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
	case WM_RECOEVENT:
		{
			ProcessRecoEvent(hwnd);
			return 0;
		}
	case WM_PLAY:
		{
			MessageBox(hwndPlayer, "Play", "Debug", MB_OK);
			break;
		}
	case WM_STOP:
		{
			MessageBox(hwndPlayer, "Stop", "Debug", MB_OK);
			break;
		}
	case WM_PAUSE:
		{
			MessageBox(hwndPlayer, "Pause", "Debug", MB_OK);
			break;
		}
	case WM_ADVANCE:
		{
			MessageBox(hwndPlayer, "Advance", "Debug", MB_OK);
			break;
		}
	case WM_PREVIOUS:
		{
			MessageBox(hwndPlayer, "Previous", "Debug", MB_OK);
			break;
		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void ProcessRecoEvent(HWND hwnd)
{
	// Event helper class
	CSpEvent event; 

	// Loop processing events while there are any in the queue
	while (event.GetFrom(RecoCtxt) == S_OK)
	{
		// Look at recognition event only
		switch (event.eEventId)
		{
			case SPEI_RECOGNITION:
				{
					ExecuteCommand(event.RecoResult(), hwnd);
					break;
				}
		}
	}
}

void ExecuteCommand(ISpPhrase *phrase, HWND hwnd)
{
	SPPHRASE *elements;

	// Get the phrase elements, one of which is the rule id we specified in
	// the grammar. Switch on it to figure out which command was recognized.
	if (SUCCEEDED(phrase->GetPhrase(&elements)))
	{ 
		switch (elements->Rule.ulId)
		{
			case PLAY:
				{
					PostMessage(hwnd, WM_PLAY, 0, 0);
					break;
				}
			case STOP:
				{
					PostMessage(hwnd, WM_STOP, 0, 0);
					break;
				}
			case PAUSE:
				{
					PostMessage(hwnd, WM_PAUSE, 0, 0);
					break;
				}
			case ADVANCE:
				{
					PostMessage(hwnd, WM_ADVANCE, 0, 0);
					break;
				}
			case PREVIOUS:
				{
					PostMessage(hwnd, WM_PREVIOUS, 0, 0);
					break;
				}
		}
	// Free the pElements memory which was allocated for us
	::CoTaskMemFree(elements);
	}
}

DWORD WINAPI ThreadProc(void* threadflags)
{
	if(init)
	{
		//MessageBox(hwndPlayer, "CoInit NULL", "debug", MB_OK);
		if (FAILED(CoInitialize(NULL)))
		{
			DestroyWindow(hVoice);
			return FALSE;
		}
		//MessageBox(hwndPlayer, "CoInit SharedRec", "debug", MB_OK);
		HRESULT hr = Engine.CoCreateInstance(CLSID_SpSharedRecognizer);
		if (FAILED(hr))
		{
			DestroyWindow(hVoice);
			CoUninitialize();
			return FALSE;
		}
		//MessageBox(hwndPlayer, "CoInit RecoCtxt", "debug", MB_OK);
		// create the command recognition context
		hr = Engine->CreateRecoContext(&RecoCtxt );
		if (FAILED(hr))
		{
			DestroyWindow(hVoice);
			CoUninitialize();
			return FALSE;
		}
		
		hr = RecoCtxt->SetNotifyWindowMessage(hVoice, WM_RECOEVENT, 0, 0);
		if (FAILED(hr))
		{
			DestroyWindow(hVoice);
			CoUninitialize();
			return FALSE;
		}
		
		hr = RecoCtxt->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
			if (FAILED(hr)) 
			{
				DestroyWindow(hVoice);
				CoUninitialize();
				return FALSE;
			}

		// Load our grammar
		// user defined
		//("GRAMMAR") resource type.
		//MessageBox(hwndPlayer, "CoInit CreateGramm", "debug", MB_OK);
		hr = RecoCtxt->CreateGrammar(0, &CmdGrammar);
		if (FAILED(hr))
		{
			DestroyWindow(hVoice);
			CoUninitialize();
			return FALSE;
		}

		//MessageBox(hwndPlayer, "Load Cmd", "debug", MB_OK);
		hr = CmdGrammar->LoadCmdFromResource(
			NULL,
			MAKEINTRESOURCEW(IDR_QCDGRAMMAR),
			L"SRGRAMMAR",
			MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL), SPLO_STATIC);

		if ( FAILED( hr ) ) //Leave application
		{
			DestroyWindow(hVoice);
			UnregisterClass(classname.c_str(), hInstance);
			CoUninitialize();
			//ShutDown(0);
			return FALSE;
		}

		hr = CmdGrammar->SetRuleState( NULL, NULL, SPRS_ACTIVE );
		if (FAILED(hr))
		{
			DestroyWindow(hVoice);
			CoUninitialize();
			return FALSE;
		}

		ExitThread(0);
	}

	if(shutdown)
	{
		CoUninitialize();
		ExitThread(0);
	}

	if(processreco)
	{
		ProcessRecoEvent(hwnd);
		ExitThread(0);
	}



