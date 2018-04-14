#include "stdafx.h"

#include <StateMachine/Assert.h>
#include "MyContext.h"

MyContext::MyContext()
{
	tsm::IContext::onAssertFailedProc = MyLogger::onAssertFailed;
}

tsm::IStateMonitor* MyContext::_getStateMonitor()
{
	return &m_logger;
}

void MyContext::createStateMachine(HWND hWnd, UINT msg)
{
	m_stateMachine.reset(tsm::IStateMachine::create(hWnd, msg));
}
