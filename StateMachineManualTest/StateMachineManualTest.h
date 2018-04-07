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

	void createStateMachine(HWND hWnd, UINT msg) { m_stateMachine.reset(tsm::IStateMachine::create(hWnd, msg)); }
protected:
	MyLogger m_logger;
};

class MyEvent : public tsm::Event
{
public:
};

class MyState : public tsm::State<MyContext, MyEvent, MyState>
{
};

class StateA : public MyState
{
public:
};

class InitialState : public MyState
{
public:
	virtual HRESULT handleEvent(MyContext*, MyEvent* event, MyState** nextState) {
		*nextState = new StateA();
		return S_OK;
	}
	virtual HRESULT entry(MyContext* context, MyEvent* event, MyState* previousState) override {
		return context->triggerEvent(new MyEvent());
	}
};
