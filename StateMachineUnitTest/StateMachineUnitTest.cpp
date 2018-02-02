// StateMachineUnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <StateMachine/Context.h>
#include <StateMachine/State.h>
#include <StateMachine/Event.h>
#include <StateMachine/StateMachine.h>

#include <iostream>

template<class T>
static LPCSTR getObjectName(T* obj) { return typeid(*obj).name(); }

class State;
class Event;

class Context : public tsm::Context<State>
{
public:
	HRESULT setup();
	HRESULT handleEvent(Event* event);

	LPCTSTR x() { return __FUNCTIONW__; }

protected:
	tsm::StateMachine sm;
};

class Event : public tsm::Event {};

class State : public tsm::State<Context, Event, State>
{
};

class InitialState : public State {
	virtual HRESULT entry(Context* context, Event* event, State* previousState) {
		std::cout << __FUNCTION__ << ": previousState=" << getObjectName(previousState) << std::endl;
		return S_OK;
	}
};

class UnInitializedState : public State
{
public:
	virtual HRESULT handleEvent(Context* context, Event* event, State** nextState) {
		std::wcout << __FUNCTIONW__ L"(" << context->x() << L"," << event->toString().c_str() << L")" << std::endl;
		*nextState = new InitialState();
		return S_OK;
		//return E_ABORT;
	}
	virtual HRESULT exit(Context* context, Event* event, State* nextState) {
		std::cout << __FUNCTION__ << ": nextState=" << getObjectName(nextState) << std::endl;
		return S_OK;
	}
};

HRESULT Context::setup()
{
	auto e = new UnInitializedState();
	return sm.setup(this, e);
}
HRESULT Context::handleEvent(Event* event)
{
	return sm.handleEvent(this, event);
}


int main()
{
	Context c;
	c.setup();
	std::cout << "Current state=" << typeid(*c.getCurrentState()).name() << std::endl;
	c.handleEvent(new Event());
	std::cout << "Current state=" << typeid(*c.getCurrentState()).name() << std::endl;

    return 0;
}
