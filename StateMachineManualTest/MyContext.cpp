#include "stdafx.h"

#include <StateMachine/Assert.h>
#include "MyContext.h"
#include "MyState.h"
#include "StateMachineManualTest.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("MyContext"));

MyContext::MyContext()
{
	tsm::IContext::onAssertFailedProc = MyContext::onAssertFailed;
}

void MyContext::createStateMachine(HWND hWnd)
{
	m_stateMachine.reset(tsm::IStateMachine::create(hWnd, WM_TRIGGER_EVENT));
	m_hWnd = hWnd;

	log(_T(__FUNCTION__) _T("(hWnd=0x%p): State machine=0x%p"), hWnd, m_stateMachine.get());
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

void MyContext::log(LPCTSTR fmt, ...)
{
	static TCHAR msg[0x100];
	va_list args;
	va_start(args, fmt);
	_vstprintf_s(msg, fmt, args);

	if(IsWindow(m_hWnd)) {
		std::unique_ptr<LogMessage> logMessage(new LogMessage());
		logMessage->msg = msg;

		GetLocalTime(&logMessage->time);
		logMessage->thread = GetCurrentThreadId();

		auto hr = WIN32_EXPECT(PostMessage(m_hWnd, WM_LOG_MESSAGE, 0, (LPARAM)logMessage.get()));
		if(SUCCEEDED(hr)) logMessage.release();
	} else {
		LOG4CPLUS_INFO(logger, msg);
	}
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
	log(msg);
}

void MyContext::onEventTriggered(tsm::IContext* context, tsm::IEvent* event)
{
	log(_T("Trigger event %s, delay=%d, interval=%d"),
		toString(event), event->_getDelayTime(), event->_getIntervalTime());
}

void MyContext::onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
{
	log(_T("Handling event %s in state %s"), toString(event), toString(current));
}

void MyContext::onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
{
	if(m_hWnd) {
		WIN32_EXPECT(PostMessage(m_hWnd, WM_STATE_CHANGED, 0, 0L));
	}

	log(_T("State changed from %s to %s"), toString(previous), toString(next));
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
