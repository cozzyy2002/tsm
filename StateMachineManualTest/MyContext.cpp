#include "stdafx.h"

#include <StateMachine/Assert.h>
#include "MyContext.h"
#include "StateMachineManualTest.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("MyContext"));

MyContext::MyContext()
{
	tsm::IContext::onAssertFailedProc = MyContext::onAssertFailed;
}

void MyContext::createStateMachine(HWND hWnd, UINT msg)
{
	m_stateMachine.reset(tsm::IStateMachine::create(hWnd, msg));
	m_hWnd = hWnd;
}


template<class T>
LPCTSTR MyContext::toString(T* obj)
{
	auto _obj = dynamic_cast<MyObject*>(obj);
	return _obj ? _obj->toString() : _T("<nullptr>");
}

void MyContext::onIdle(tsm::IContext* context)
{
	LOG4CPLUS_INFO(logger, "StateMachined is idle.");
}

void MyContext::onEventTriggered(tsm::IContext* context, tsm::IEvent* event)
{
}

void MyContext::onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
{
	LOG4CPLUS_INFO(logger, "Handling event " << toString(event) << " in state " << toString(current));
}

void MyContext::onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
{
	LOG4CPLUS_INFO(logger, "State changed from " << toString(previous) << " to " << toString(next));

	PostMessage(m_hWnd, WM_STATE_CHANGED, 0, 0);
}

void MyContext::onTimerStarted(tsm::IContext* context, tsm::IEvent* event)
{
}

void MyContext::onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode)
{
	LOG4CPLUS_INFO(logger, "Worker thread exit. Code=0x" << std::hex << exitCode);
}

void MyContext::onAssertFailed(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	LOG4CPLUS_ERROR(logger, exp << " failed. HRESULT=0x" << std::hex << hr << ". at:\n" << sourceFile << "#L" << std::dec << line);
}
