// StateMachineManualTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "StateMachineManualTest.h"
#include "MyContext.h"
#include "MyEvent.h"
#include "MyState.h"
#include "ReportView.h"

#include <StateMachine/Assert.h>

#include <log4cplus/configurator.h>
#include <memory>
#include <CommCtrl.h>

static MyContext context;
static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("App main"));

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWndTriggerEventDlg = NULL;				// Window hande of trigger event dialog.

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK    triggerEventDialogProc(HWND, UINT, WPARAM, LPARAM);
static HRESULT triggerEvent(HWND hDlg);
static MyEvent* createEvent(HWND hDlg);
static HRESULT onWmNotify(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static DWORD getEditNumeric(HWND hDlg, int id);
static std::tstring getEditText(HWND hDlg, int id);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	log4cplus::PropertyConfigurator::doConfigure(_T("log4cplus.properties"));

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_STATEMACHINEMANUALTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_STATEMACHINEMANUALTEST));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STATEMACHINEMANUALTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_STATEMACHINEMANUALTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

static BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	context.setLogWindow(hInst, hwnd, WM_LOG);

	return TRUE;
}

static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	// Parse the menu selections:
	switch(id)
	{
	case IDM_TRIGGER_EVENT:
		if(!hWndTriggerEventDlg) {
			hWndTriggerEventDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_EVENT), hwnd, triggerEventDialogProc);
		}
		ShowWindow(hWndTriggerEventDlg, SW_SHOW);
		break;
	case IDM_VIEW_CLEAR_LOG:
		context.getReportView()->clear();
		break;
	case IDM_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
		break;
	case IDM_EXIT:
		DestroyWindow(hwnd);
		break;
	default:
		FORWARD_WM_COMMAND(hwnd, id, hwndCtl, codeNotify, DefWindowProc);
		break;
	}
}

static void OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	// TODO: Add any drawing code that uses hdc here...
	EndPaint(hwnd, &ps);
}

static void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	context.getReportView()->resize(rect.right - rect.left, rect.bottom - rect.top);
}

static void OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

// Message cracker for WM_LOG to call MyContext::onLogMsg().
#define HANDLE_WM_LOG(hwnd, wParam, lParam, fn) \
	((fn)((hwnd), (wParam), (lParam)), 0L)

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hWnd, WM_SIZE, OnSize);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_LOG, context.onLogMsg);
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

static BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	context.createStateMachine(hwnd, WM_STATE_MACHINE);

	// Disable trigger event button until event name will be entered.
	Button_Enable(GetDlgItem(hwnd, IDC_BUTTON_TRIGGER_EVENT), FALSE);

	Button_SetCheck(GetDlgItem(hwnd, IDC_RADIO_TIMER_STATE), TRUE);
	return TRUE;
}

static void OnDlgCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id) {
	case IDC_BUTTON_SETUP:
		HR_EXPECT_OK(context.setup(new MyState(_T("Initial"))));
		break;
	case IDC_BUTTON_SHUTDOWN:
		HR_EXPECT_OK(context.shutdown());
		break;
	case IDC_BUTTON_TRIGGER_EVENT:
		HR_EXPECT_OK(triggerEvent(hwnd));
		break;
	case IDCANCEL:
		// [Close] button is pressed.
		ShowWindow(hwnd, SW_HIDE);
		break;
	case IDC_EDIT_EVENT_NAME:
		switch(codeNotify) {
		case EN_CHANGE:
			// Enable trigger event button if next state name is entered.
			Button_Enable(GetDlgItem(hwnd, IDC_BUTTON_TRIGGER_EVENT), 0 < Edit_GetTextLength(hwndCtl));
			break;
		}
		break;
	case IDC_EDIT_NEXT_STATE_NAME:
		switch(codeNotify) {
		case EN_CHANGE:
			{
				// If master state of next state exist, set it's name to master state edit box.
				auto state = context.findState(getEditText(hwnd, IDC_EDIT_NEXT_STATE_NAME));
				auto masterStateEneble = TRUE;
				auto hWndMasterState = GetDlgItem(hwnd, IDC_EDIT_MASTER_STATE_NAME);
				if(state && state->getMasterState()) {
					Edit_SetText(hWndMasterState, state->getMasterState()->getName().c_str());
					masterStateEneble = FALSE;
				}
				Edit_Enable(hWndMasterState, masterStateEneble);
			}
			return;
		}
		// Go below
	case IDC_EDIT_MASTER_STATE_NAME:
		//LOG4CPLUS_INFO(logger, "id=" << id << ", codeNotify=" << codeNotify);
		onWmNotify(hwnd, id, hwndCtl, codeNotify);
		break;
	}
}

static LRESULT OnDlgNotify(HWND hWnd, int idForm, NMHDR* nmhdr)
{
	LOG4CPLUS_INFO(logger, "WM_NOTIFY: ID=" << nmhdr->idFrom << ", code=" << nmhdr->code);
	return onWmNotify(hWnd, nmhdr->idFrom, nmhdr->hwndFrom, nmhdr->code);
}

#define HANDLE_DLG_MSG(hwnd, msg, fn) \
	case msg: return SetDlgMsgResult(hwnd, msg, HANDLE_##msg(hwnd, wParam, lParam, fn));

INT_PTR CALLBACK    triggerEventDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch(message)
	{
		HANDLE_DLG_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
		HANDLE_DLG_MSG(hDlg, WM_COMMAND, OnDlgCommand);
		HANDLE_DLG_MSG(hDlg, WM_NOTIFY, OnDlgNotify);
	}
	return (INT_PTR)FALSE;
}

/*static*/ HRESULT triggerEvent(HWND hDlg)
{
	auto event = createEvent(hDlg);
	return event ? context.triggerEvent(createEvent(hDlg)) : S_FALSE;
}

/*static*/ MyEvent* createEvent(HWND hDlg)
{
	auto event = new MyEvent(getEditText(hDlg, IDC_EDIT_EVENT_NAME));
	auto stateName = getEditText(hDlg, IDC_EDIT_NEXT_STATE_NAME);
	if(!stateName.empty()) {
		// Set next state.
		auto state = context.findState(stateName);
		event->nextState = state ? state : new MyState(stateName);
		stateName = getEditText(hDlg, IDC_EDIT_MASTER_STATE_NAME);
		if(!stateName.empty()) {
			// Set master state of next state.
			auto masterState = context.findState(stateName);
			event->nextState->setMasterState(
				masterState ? masterState : new MyState(stateName));
		}
	}
	event->hrHandleEvent = getEditNumeric(hDlg, IDC_EDIT_HR_HANDLE_EVENT);
	event->hrEntry = getEditNumeric(hDlg, IDC_EDIT_HR_ENTRY);
	event->hrExit = getEditNumeric(hDlg, IDC_EDIT_HR_EXIT);
	auto delay = getEditNumeric(hDlg, IDC_EDIT_DELAY_TIME);
	if(delay) {
		auto interval = getEditNumeric(hDlg, IDC_EDIT_INTERVAL_TIME);
		bool isContextTimer = (BST_CHECKED == Button_GetCheck(GetDlgItem(hDlg, IDC_RADIO_TIMER_CONTEXT)));
		auto timerClient = isContextTimer ? (tsm::TimerClient*)&context : context.getCurrentState();
		event->setTimer(timerClient, delay, interval);
	}
	return event;
}

/*static*/ HRESULT onWmNotify(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	auto handleMessage = false;
	switch(codeNotify) {
	case EN_SETFOCUS:
		switch(id) {
		case IDC_EDIT_NEXT_STATE_NAME:
		case IDC_EDIT_MASTER_STATE_NAME:
			// If edit control is empty, handle this message.
			handleMessage = (0 == Edit_GetTextLength(hwndCtl));
			break;
		}
		break;
	}

	if(handleMessage) {
		auto hWndStates = GetDlgItem(hwnd, IDC_LIST_STATES);
		auto index = ListBox_GetCurSel(hWndStates);
		if(index != LB_ERR) {
			auto state = (MyState*)ListBox_GetItemData(hWndStates, index);
			Edit_SetText(hwndCtl, state->getName().c_str());
		}
		return S_OK;
	} else {
		return FORWARD_WM_NOTIFY(hwnd, id, hwndCtl, DefWindowProc);
	}
}

DWORD getEditNumeric(HWND hDlg, int id)
{
	auto text = getEditText(hDlg, id);
	if(!text.empty()) {
		TCHAR *endptr;
		return (DWORD)_tcstoul(text.c_str(), &endptr, 0);
	}
	return 0;
}

std::tstring getEditText(HWND hDlg, int id)
{
	std::tstring text;
	auto hEdit = GetDlgItem(hDlg, id);
	auto textLen = Edit_GetTextLength(hEdit);
	if(0 < textLen) {
		textLen++;
		std::unique_ptr<TCHAR[]> buff(new TCHAR[textLen]);
		Edit_GetText(hEdit, buff.get(), textLen);
		text = buff.get();
	}
	return text;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
