#pragma once

#include <StateMachine/Event.h>
#include "MyObject.h"
#include "MyState.h"

class MyEvent : public tsm::Event, public MyObject
{
public:
	MyEvent(const std::tstring& name) : MyObject(name.c_str()) {
		hrHandleEvent = hrEntry = hrExit = S_OK;
	}

	CComPtr<MyState> nextState;
	HRESULT hrHandleEvent;
	HRESULT hrEntry;
	HRESULT hrExit;
};
