#pragma once

#include <StateMachine/Event.h>
#include "MyObject.h"
#include "MyState.h"

class MyEvent : public tsm::Event, public MyObject
{
public:
	MyEvent(ILogger& logger, const std::tstring& name) : MyObject(name.c_str(), &logger) {
		hrHandleEvent = hrEntry = hrExit = S_OK;
	}

	// Inplementation of IUnknown to log deleting object.
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	CComPtr<MyState> nextState;
	HRESULT hrHandleEvent;
	HRESULT hrEntry;
	HRESULT hrExit;
};
