
// StateMachineManualTest.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "StateMachineManualTest.h"
#include "StateMachineManualTestDlg.h"

#include <StateMachine/Event.h>
#include <StateMachine/State.h>
#include <StateMachine/Assert.h>

#include <log4cplus/configurator.h>

static log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("CStateMachineManualTestApp"));

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CStateMachineManualTestApp

BEGIN_MESSAGE_MAP(CStateMachineManualTestApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CStateMachineManualTestApp construction

CStateMachineManualTestApp::CStateMachineManualTestApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CStateMachineManualTestApp object

CStateMachineManualTestApp theApp;


// CStateMachineManualTestApp initialization

BOOL CStateMachineManualTestApp::InitInstance()
{
	log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("log4cplus.properties"));

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("StateMachineManualTest"));

	CStateMachineManualTestDlg dlg;
	m_pMainWnd = &dlg;
	context.setLogWindow(this->m_hInstance, m_pMainWnd->GetSafeHwnd(), 200);

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		LOG4CPLUS_WARN(logger, "Dialog create failed. CDialog::DoModal() returned " << nResponse);

		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
