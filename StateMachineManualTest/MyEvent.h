#pragma once

#include <StateMachine/Event.h>
#include "MyObject.h"
#include "MyState.h"

class MyEvent : public tsm::Event<MyContext>, public MyObject
{
public:
	MyEvent(ILogger& logger, const std::tstring& name, int priority);
	~MyEvent();

	// Implementation of tsm::Event
	virtual HRESULT preHandle(MyContext* context) override;
	virtual HRESULT postHandle(MyContext* context, HRESULT hr) override;

	CComPtr<MyState> nextState;
	HRESULT hrPreHandle;
	HRESULT hrPostHandle;
	HRESULT hrHandleEvent;
	HRESULT hrEntry;
	HRESULT hrExit;
};
