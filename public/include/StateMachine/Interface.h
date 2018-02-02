#pragma once

#include <StateMachine/Unknown.h>

namespace tsm {

class IState;

class IContext
{
public:
	virtual ~IContext() {}

	CComPtr<IState> m_currentState;
};

class IEvent : public Unknown
{
public:
	virtual ~IEvent() {}
};

class IState : public Unknown
{
public:
	IState(IState* masterState) : m_masterState(masterState) {}
	virtual ~IState() {}

#pragma region Methods to be called by StateMachine.
	virtual HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) = 0;
	virtual HRESULT _entry(IContext* context, IEvent* event, IState* previousState) = 0;
	virtual HRESULT _exit(IContext* context, IEvent* event, IState* nextState) = 0;
#pragma endregion

	CComPtr<IState> m_subState;
	IState* m_masterState;
};
}
