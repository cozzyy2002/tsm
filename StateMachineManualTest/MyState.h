#pragma once

#include <StateMachine/State.h>
#include "MyObject.h"

class MyContext;
class MyEvent;
class MyState;

class MyState : public tsm::State<MyContext, MyEvent, MyState>, public MyObject
{
public:
	MyState(ILogger& logger, const std::tstring& name, MyState* masterState = nullptr)
		: State(masterState), MyObject(name.c_str(), &logger), callExitOnShutDown(false) {}

	// Inplementation of IUnknown to log deleting object.
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	virtual HRESULT handleEvent(MyContext*, MyEvent* event, MyState** nextState) override;
	virtual HRESULT entry(MyContext* context, MyEvent* event, MyState* previousState) override;
	virtual HRESULT exit(MyContext* context, MyEvent* event, MyState* nextState) override;
	virtual bool _callExitOnShutdown() const override { return callExitOnShutDown; }
	bool callExitOnShutDown;

	void setMasterState(MyState* masterState) { m_masterState = masterState; }
};
