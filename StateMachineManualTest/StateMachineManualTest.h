#pragma once

#include "resource.h"

#include <StateMachine/Context.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>
#include <StateMachine/Assert.h>

class MyState;
class MyEvent;

#include "MyLogger.h"

class MyContext : public tsm::AsyncContext<MyEvent, MyState>
{
public:
	MyContext() {
		tsm::IContext::onAssertFailedProc = MyLogger::onAssertFailed;
	}

	virtual tsm::IStateMonitor* _getStateMonitor() override { return &m_logger; }

protected:
	MyLogger m_logger;
};

class MyEvent : public tsm::Event
{
public:
};

class MyState : public tsm::State<MyContext, MyEvent, MyState>
{
public:
};

class InitialState : public MyState
{};
