
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



CReportView& CStateMachineManualTestDlg::getActivaReportView()
{
	int ctrlID = 0;
	auto ctrl = GetFocus();
	if(ctrl) {
		ctrlID = ctrl->GetDlgCtrlID();
	}
	switch(ctrlID) {
	default:
	case IDC_LIST_LOG:
		return m_logView;
	case IDC_LIST_STATES:
		// Copy current state to clipboard.
		return m_statesView;
	}
}

CStateMachineManualTestDlg::CStateMachineManualTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_STATEMACHINEMANUALTEST_DIALOG, pParent), m_eventProperties(&context)
	, m_logNo(0)
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
ON_MESSAGE(WM_STATE_CHANGED, &CStateMachineManualTestDlg::OnStateChanged)
//ON_COMMAND(IDOK, &CStateMachineManualTestDlg::OnIdok)
//ON_COMMAND(IDCANCEL, &CStateMachineManualTestDlg::OnIdcancel)
ON_COMMAND(ID_FILE_EXIT, &CStateMachineManualTestDlg::OnFileExit)
ON_COMMAND(ID_EDIT_SELECT_ALL, &CStateMachineManualTestDlg::OnEditSelectAll)
ON_COMMAND(ID_EDIT_COPY, &CStateMachineManualTestDlg::OnEditCopy)
ON_COMMAND(ID_EDIT_CLEAR, &CStateMachineManualTestDlg::OnEditClear)
ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, &CStateMachineManualTestDlg::OnUpdateEditClear)
ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CStateMachineManualTestDlg::OnUpdateEditCopy)
END_MESSAGE_MAP()


static TCHAR logTimeFormat[] = _T("%02d:%02d:%02d.%03d");
static TCHAR logTimeBuff[] = _T("hh:mm:ss.xxx");

static const CReportView::Column logColumns[] = {
	{ CReportView::Column::Type::Number, _T("No."), 10 },
	{ CReportView::Column::Type::Number, _T("Time"), ARRAYSIZE(logTimeBuff) },
	{ CReportView::Column::Type::Number, _T("Thread"), 10 },
	{ CReportView::Column::Type::String, _T("Message"), CReportView::remainingColumnWidth },
};

static const CReportView::Column statesColumns[] = {
	{ CReportView::Column::Type::String, _T("Name"), CReportView::remainingColumnWidth },
	{ CReportView::Column::Type::Number, _T("Address"), (sizeof(void*) * 2) + 5 },
	{ CReportView::Column::Type::Bool, _T("isSubState"), CReportView::autoColumnWidth },
	{ CReportView::Column::Type::Bool, _T("callExitOnShutdown"), CReportView::autoColumnWidth },
};

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

	m_hAccel = LoadAccelerators(theApp.m_hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	HR_EXPECT_OK(m_logView.setColumns(m_listLog.m_hWnd, logColumns));
	HR_EXPECT_OK(m_statesView.setColumns(m_listStates.m_hWnd, statesColumns));

	context.createStateMachine(this->m_hWnd);

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
	::DestroyAcceleratorTable(m_hAccel);

	CDialogEx::OnDestroy();
}


afx_msg LRESULT CStateMachineManualTestDlg::OnLogMessage(WPARAM wParam, LPARAM lParam)
{
	std::unique_ptr<LogMessage> logMessage((LogMessage*)lParam);
	auto& st = logMessage->time;
	_stprintf_s(logTimeBuff, logTimeFormat, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	const CVar items[ARRAYSIZE(logColumns)] = {
		CVar(++m_logNo), CVar(logTimeBuff), CVar((int)logMessage->thread), CVar(logMessage->msg)
	};
	m_logView.addItems(items);
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
	UpdateData();
	auto e = m_eventProperties.createEvent();
	HR_EXPECT_OK(context.triggerEvent(e));
}


afx_msg LRESULT CStateMachineManualTestDlg::OnStateChanged(WPARAM wParam, LPARAM lParam)
{
	m_eventProperties.updateStates();

	m_statesView.clear();
	for(auto state = context.getCurrentState(); state; state = state->getMasterState()) {
		const CVar items[ARRAYSIZE(statesColumns)] = {
			CVar(state->MyObject::getName()),
			CVar((MyObject*)state),		// In log window, address of state object is shown as pointer of MyObject class.
			CVar(state->isSubState()),
			CVar(state->_callExitOnShutdown()),
		};
		m_statesView.addItems(items, state);
	}

	return 0;
}


void CStateMachineManualTestDlg::OnFileExit()
{
	CDialogEx::EndDialog(IDOK);
}


void CStateMachineManualTestDlg::OnOK()
{
	// Ignore [Enter]
	// Use [File]->[Exit] to terminate.
	//CDialogEx::OnOK();
}


void CStateMachineManualTestDlg::OnCancel()
{
	// Ignore [Esc]
	// Use [File]->[Exit] to terminate.
	//CDialogEx::OnCancel();
}


void CStateMachineManualTestDlg::OnEditSelectAll()
{
	// TODO: Add your command handler code here
}


void CStateMachineManualTestDlg::OnEditCopy()
{
	getActivaReportView().copy();
}


void CStateMachineManualTestDlg::OnEditClear()
{
	m_logView.clear();
}


BOOL CStateMachineManualTestDlg::PreTranslateMessage(MSG* pMsg)
{
	if(m_hAccel) return ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg);

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CStateMachineManualTestDlg::OnUpdateEditClear(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(0 < m_logView.getItemCount());
}


void CStateMachineManualTestDlg::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(0 < getActivaReportView().getItemCount());
}
