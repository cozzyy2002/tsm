#pragma once

#include <StateMachine/State.h>
#include "MyObject.h"

class MyContext;
class MyEvent;
class MyState;

class MyState : public tsm::State<MyContext, MyEvent, MyState>, public MyObject
{
public:
	MyState(ILogger& logger, const std::tstring& name, MyState* masterState = nullptr);
	~MyState();

	virtual HRESULT handleEvent(MyContext*, MyEvent* event, MyState** nextState) override;
	virtual HRESULT entry(MyContext* context, MyEvent* event, MyState* previousState) override;
	virtual HRESULT exit(MyContext* context, MyEvent* event, MyState* nextState) override;
	virtual bool _isExitCalledOnShutdown() const override { return isExitCalledOnShutdown; }
	bool isExitCalledOnShutdown;

	void setMasterState(MyState* masterState) { m_masterState = masterState; }
};
