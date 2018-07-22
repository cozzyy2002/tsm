
// StateMachineManualTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "StateMachineManualTest.h"
#include "StateMachineManualTestDlg.h"
#include "afxdialogex.h"
#include "MyEvent.h"
#include "MyState.h"
#include <StateMachine/Assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CStateMachineManualTestDlg dialog



CStateMachineManualTestDlg::CStateMachineManualTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_STATEMACHINEMANUALTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStateMachineManualTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_LIST_STATES, m_listStates);
	DDX_Control(pDX, IDC_EVENT_PROPERTIES, m_eventProperties);
}

BEGIN_MESSAGE_MAP(CStateMachineManualTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_STATECONTROL, &CStateMachineManualTestDlg::OnFileStatecontrol)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_LOG_MESSAGE, &CStateMachineManualTestDlg::OnLogMessage)
//	ON_WM_CREATE()
ON_BN_CLICKED(IDC_BUTTON_SETUP, &CStateMachineManualTestDlg::OnClickedButtonSetup)
ON_BN_CLICKED(IDC_BUTTON_SHUTDOWN, &CStateMachineManualTestDlg::OnClickedButtonShutdown)
ON_BN_CLICKED(IDC_BUTTON_TRIGGER_EVENT, &CStateMachineManualTestDlg::OnClickedButtonTriggerEvent)
END_MESSAGE_MAP()


// CStateMachineManualTestDlg message handlers

BOOL CStateMachineManualTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	context.setLogWindow(m_listLog.m_hWnd, WM_LOG_MESSAGE);
	context.setStatesView(m_listStates.m_hWnd);

	context.createStateMachine(this->m_hWnd, WM_TRIGGER_EVENT);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CStateMachineManualTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CStateMachineManualTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CStateMachineManualTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CStateMachineManualTestDlg::OnFileStatecontrol()
{
	context.log(_T(__FUNCTION__) _T(": HWND=0x%p"), this->m_hWnd);
}


void CStateMachineManualTestDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

}


afx_msg LRESULT CStateMachineManualTestDlg::OnLogMessage(WPARAM wParam, LPARAM lParam)
{
	context.onLogMsg(this->m_hWnd, wParam, lParam);
	return 0;
}


void CStateMachineManualTestDlg::OnClickedButtonSetup()
{
	HR_EXPECT_OK(context.setup(new MyState(context, _T("Initial state"))));
}


void CStateMachineManualTestDlg::OnClickedButtonShutdown()
{
	HR_EXPECT_OK(context.shutdown());
}


void CStateMachineManualTestDlg::OnClickedButtonTriggerEvent()
{
}
