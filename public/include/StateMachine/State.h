#pragma once

#include "Interface.h"
#include "TimerClient.h"

namespace tsm {

template<class C = IContext, class E = IEvent, class S = IState>
class State : public IState, public TimerClient
{
public:
	State(IState* masterState = nullptr)
		: m_masterState(masterState)
		, m_handle(HandleFactory<IState, StateHandle>::create(this)) {}
	virtual ~State() {}

#pragma region Implementation of IState that call methods of sub class.
	HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) override {
		return handleEvent((C*)context, (E*)event, (S**)nextState);
	}
	HRESULT _entry(IContext* context, IEvent* event, IState* previousState) override {
		return entry((C*)context, (E*)event, (S*)previousState);
	}
	HRESULT _exit(IContext* context, IEvent* event, IState* nextState) override {
		return exit((C*)context, (E*)event, (S*)nextState);
	}

	virtual bool _isExitCalledOnShutdown() const override { return false; }
	virtual IState* _getMasterState() const override { return m_masterState; }

	virtual TimerClient* _getTimerClient() override { return this; }
	virtual StateHandle* _getHandle() override { return m_handle.get(); }
#pragma endregion

#pragma region Methods to be implemented by sub class.
	virtual HRESULT handleEvent(C*, E* event, S** nextState) { return S_FALSE; }
	virtual HRESULT entry(C* context, E* event, S* previousState) { return S_FALSE; }
	virtual HRESULT exit(C* context, E* event, S* nextState) { return S_FALSE; }
#pragma endregion

	S* getMasterState() { return (S*)m_masterState.p; }
	S* getSubState() { return (S*)m_subState.p; }

	bool isSubState() const { return m_masterState ? true : false; }

protected:
	std::unique_ptr<StateHandle, HandleFactory<IState, StateHandle>> m_handle;
	CComPtr<IState> m_masterState;
};

}
