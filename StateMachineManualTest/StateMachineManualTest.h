
// StateMachineManualTest.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "MyContext.h"

#include <log4cplus/initializer.h>

// CStateMachineManualTestApp:
// See StateMachineManualTest.cpp for the implementation of this class
//

class CStateMachineManualTestApp : public CWinApp
{
protected:
	MyContext context;

public:
	CStateMachineManualTestApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
//	virtual int ExitInstance();

private:
	// Initialize and ShutDown log4cplus.
	// This member is never used.
	log4cplus::Initializer _log4cplus_Initializer;
};

extern CStateMachineManualTestApp theApp;