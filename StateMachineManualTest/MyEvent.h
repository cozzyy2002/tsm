#pragma once

#include <StateMachine/Event.h>
#include "MyObject.h"
#include "MyState.h"

class MyEvent : public tsm::Event<MyContext>, public MyObject
{
public:
	MyEvent(ILogger& logger, const std::tstring& name, int priority)
		: MyObject(name.c_str(), &logger), Event(priority) {
		hrHandleEvent = hrEntry = hrExit = S_OK;
	}

	// Implementation of tsm::Event
	virtual HRESULT preHandle(MyContext* context) override;

	// Inplementation of IUnknown to log deleting object.
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	CComPtr<MyState> nextState;
	HRESULT hrPreHandle;
	HRESULT hrHandleEvent;
	HRESULT hrEntry;
	HRESULT hrExit;
};
