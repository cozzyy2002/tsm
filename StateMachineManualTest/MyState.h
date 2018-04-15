#pragma once

#include <StateMachine/State.h>
#include "MyObject.h"

class MyContext;
class MyEvent;
class MyState;

using MyStateBase = tsm::State<MyContext, MyEvent, MyState>;

class MyState : public MyStateBase, public MyObject
{
public:
	MyState(const std::tstring& name, MyState* masterState = nullptr) : MyStateBase(masterState), MyObject(name.c_str()) {}

	virtual HRESULT handleEvent(MyContext*, MyEvent* event, MyState** nextState) override;
	virtual HRESULT entry(MyContext* context, MyEvent* event, MyState* previousState) override;
	virtual HRESULT exit(MyContext* context, MyEvent* event, MyState* nextState) override;
};

class StateA : public MyState
{
public:
	StateA() : MyState(_T("State A")) {}
};

class InitialState : public MyState
{
public:
	InitialState() : MyState(_T("Initial")) {}
	virtual HRESULT handleEvent(MyContext*, MyEvent* event, MyState** nextState) override;
	virtual HRESULT entry(MyContext* context, MyEvent* event, MyState* previousState) override;
};
