#pragma once

#include "Interface.h"
#include "TimerClient.h"
#include <StateMachine/Assert.h>

namespace tsm {

template<class C = IContext, class E = IEvent>
class State : public IState, public TimerClient
{
public:
	State(IState* masterState = nullptr) : m_masterState(masterState), m_entryCalled(false) {}
	virtual ~State() {}

#pragma region Implementation of IState that call methods of sub class.
	HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) override {
		return handleEvent((C*)context, (E*)event, nextState);
	}
	HRESULT _entry(IContext* context, IEvent* event, IState* previousState) override {
		m_entryCalled = true;
		return entry((C*)context, (E*)event, previousState);
	}
	HRESULT _exit(IContext* context, IEvent* event, IState* nextState) override {
		m_entryCalled = false;
		HR_ASSERT_OK(cancelAllEventTimers());
		return exit((C*)context, (E*)event, nextState);
	}

	virtual IState* _getMasterState() const override { return m_masterState; }
	virtual void _setMasterState(IState* state) override { m_masterState = state; }
	virtual bool _hasEntryCalled() const override { return m_entryCalled; }
#pragma endregion

#pragma region Methods to be implemented by sub class.
	virtual HRESULT handleEvent(C*, E* event, IState** nextState) { return S_FALSE; }
	virtual HRESULT entry(C* context, E* event, IState* previousState) { return S_FALSE; }
	virtual HRESULT exit(C* context, E* event, IState* nextState) { return S_FALSE; }
#pragma endregion

	State* getMasterState() { return (State*)m_masterState.p; }
	State* getSubState() { return (State*)m_subState.p; }

	bool isSubState() const { return m_masterState ? true : false; }

protected:
	CComPtr<IState> m_masterState;
	bool m_entryCalled;
};

}
