#include "stdafx.h"

#include <StateMachine/Assert.h>
#include "MyContext.h"
#include "MyState.h"
#include "StateMachineManualTest.h"

static TCHAR logTimeFormat[] = _T("%02d:%02d:%02d.%03d");
static TCHAR logTimeBuff[] = _T("hh:mm:ss.xxx");

/*static*/ const CReportView::Column MyContext::m_logColumns[] = {
	{ CReportView::Column::Type::Number, _T("No."), 10 },
	{ CReportView::Column::Type::Number, _T("Time"), ARRAYSIZE(logTimeBuff) },
	{ CReportView::Column::Type::Number, _T("Thread"), 10 },
	{ CReportView::Column::Type::String, _T("Message"), CReportView::remainingColumnWidth },
};

/*static*/ const CReportView::Column MyContext::m_statesColumns[] = {
	{ CReportView::Column::Type::String, _T("Name"), CReportView::remainingColumnWidth },
	{ CReportView::Column::Type::Number, _T("Address"), (sizeof(void*) * 2) + 5 },
	{ CReportView::Column::Type::Bool, _T("isSubState"), CReportView::autoColumnWidth },
	{ CReportView::Column::Type::Bool, _T("callExitOnShutdown"), CReportView::autoColumnWidth },
};

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("MyContext"));

MyContext::MyContext()
	: m_logNo(0), m_hWndLog(NULL), m_startTime(GetTickCount())
{
	tsm::IContext::onAssertFailedProc = MyContext::onAssertFailed;
}

void MyContext::createStateMachine(HWND hWnd, UINT msg)
{
	m_stateMachine.reset(tsm::IStateMachine::create(hWnd, msg));
	m_hWnd = hWnd;
}

MyState* MyContext::findState(const std::tstring& name) const
{
	auto state = getCurrentState();
	while(state) {
		if(state->getName() == name) break;
		state = state->getMasterState();
	}
	return state;
}

void MyContext::setLogWindow(HWND hWndLog, UINT logMsg)
{
	m_hWndLog = hWndLog;
	m_logMsg = logMsg;

	HR_EXPECT_OK(m_logView.setColumns(hWndLog, m_logColumns));
}

struct LogMessage
{
	SYSTEMTIME time;
	DWORD thread;
	std::tstring msg;
};

void MyContext::log(LPCTSTR fmt, ...)
{
	if(!m_hWndLog || !IsWindow(m_hWnd)) return;

	std::unique_ptr<LogMessage> logMessage(new LogMessage());

	static TCHAR msg[0x100];
	va_list args;
	va_start(args, fmt);
	_vstprintf_s(msg, fmt, args);
	logMessage->msg = msg;

	GetLocalTime(&logMessage->time);
	logMessage->thread = GetCurrentThreadId();

	auto hr = WIN32_EXPECT(PostMessage(m_hWnd, m_logMsg, 0, (LPARAM)logMessage.get()));
	if(SUCCEEDED(hr)) logMessage.release();
}

LRESULT MyContext::onLogMsg(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	std::unique_ptr<LogMessage> logMessage((LogMessage*)lParam);
	auto& st = logMessage->time;
	_stprintf_s(logTimeBuff, logTimeFormat, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	const CVar items[ARRAYSIZE(m_logColumns)] = {
		CVar(++m_logNo), CVar(logTimeBuff), CVar((int)logMessage->thread), CVar(logMessage->msg)
	};
	m_logView.addItems(items);

	return 0;
}

void MyContext::setStatesView(HWND hWndStates)
{
	HR_EXPECT_OK(m_statesView.setColumns(hWndStates, m_statesColumns));
}

/**
 * Returns MyState object setlected in states list view.
 *
 * If no item is selected, returns nullptr.
 */
MyState* MyContext::getSelectedState() const
{
	MyState* state = nullptr;
	auto selected = m_statesView.getSelectedIndex();
	if(0 <= selected) {
		state = (MyState*)m_statesView.getItemData(selected);
	}
	return state;
}

template<class T>
LPCTSTR MyContext::toString(T* obj)
{
	auto _obj = dynamic_cast<MyObject*>(obj);
	return _obj ? _obj->toString() : _T("<nullptr>");
}

void MyContext::onIdle(tsm::IContext* context)
{
	LPCTSTR msg = _T("StateMachine is idle.");
	if(m_hWndLog) {
		log(msg);
	} else {
		LOG4CPLUS_INFO(logger, msg);
	}
}

void MyContext::onEventTriggered(tsm::IContext* context, tsm::IEvent* event)
{
	log(_T("Trigger event %s, delay=%d, interval=%d"),
		toString(event), event->_getDelayTime(), event->_getIntervalTime());
}

void MyContext::onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
{
	if(m_hWndLog) {
		log(_T("Handling event %s in state %s"), toString(event), toString(current));
	} else {
		LOG4CPLUS_INFO(logger, "Handling event " << toString(event) << " in state " << toString(current));
	}
}

void MyContext::onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
{
	if(m_hWndLog) {
		log(_T("State changed from %s to %s"), toString(previous), toString(next));
	} else {
		LOG4CPLUS_INFO(logger, "State changed from " << toString(previous) << " to " << toString(next));
	}

	m_statesView.clear();
	for(auto state = ((MyContext*)context)->getCurrentState(); state; state = state->getMasterState()) {
		const CVar items[ARRAYSIZE(m_statesColumns)] = {
			CVar(state->MyObject::getName()),
			CVar((MyObject*)state),		// In log window, address of state object is shown as pointer of MyObject class.
			CVar(state->isSubState()),
			CVar(state->_callExitOnShutdown()),
		};
		m_statesView.addItems(items, state);
	}
}

void MyContext::onTimerStarted(tsm::IContext* context, tsm::IEvent* event)
{
	log(_T("Timer started of event %s"), toString(event));
}

void MyContext::onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode)
{
	//LOG4CPLUS_INFO(logger, "Worker thread exit. Code=0x" << std::hex << exitCode);
}

/*static*/ void MyContext::onAssertFailed(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	LOG4CPLUS_ERROR(logger, exp << " failed. HRESULT=0x" << std::hex << hr << ". at:\n" << sourceFile << "#L" << std::dec << line);
}
