#pragma once

#include <StateMachine/Context.h>
#include "MyObject.h"
#include "MyLogger.h"

class MyState;
class MyEvent;

class MyContext : public tsm::AsyncContext<MyEvent, MyState>, public MyObject
{
public:
	MyContext();

	virtual tsm::IStateMonitor* _getStateMonitor() override;
	void createStateMachine(HWND hWnd, UINT msg);

protected:
	MyLogger m_logger;
};
