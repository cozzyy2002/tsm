#pragma once

#include <StateMachine/State.h>
#include "MyObject.h"

class MyContext;
class MyEvent;

class MyState : public tsm::State<MyContext, MyEvent, MyState>, public MyObject
{
};

class StateA : public MyState
{
public:
};

class InitialState : public MyState
{
public:
	virtual HRESULT handleEvent(MyContext*, MyEvent* event, MyState** nextState);
	virtual HRESULT entry(MyContext* context, MyEvent* event, MyState* previousState) override;
};
