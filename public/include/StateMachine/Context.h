#pragma once

#include "Interface.h"

#include <memory>
namespace tsm {

template<class E = IEvent, class S = IState>
class Context : public IContext
{
public:
	Context() { m_stateMachine.reset(IStateMachine::create(this)); }
	virtual ~Context() {}

	HRESULT setup(S* initialState, E* event = nullptr) { return m_stateMachine->setup(this, initialState, event); }
	HRESULT shutdown(DWORD timeout = 100) { return m_stateMachine->shutdown(this, timeout); }
	HRESULT handleEvent(E* event) { return m_stateMachine->handleEvent(this, event); }
	HRESULT waitReady(DWORD timeout = 100) { return m_stateMachine->waitReady(this, timeout); }
	S* getCurrentState() const { return (S*)m_currentState.p; }

	virtual IStateMachine* _getStateMachine() { return m_stateMachine.get(); }
	virtual IState* _getCurrentState() { return m_currentState; }
	virtual void _setCurrentState(IState* state) { m_currentState = state; }

	// Returns nullptr(Async operation is not supported).
	virtual AsyncData* _getAsyncData() { return nullptr; }

	virtual lock_t* _getHandleEventLock() { return new lock_t(m_handleEventLock); }

protected:
	std::unique_ptr<IStateMachine> m_stateMachine;
	CComPtr<IState> m_currentState;
	lock_object_t m_handleEventLock;
};

template<class E = IEvent, class S = IState>
class AsyncContext : public Context<E, S>
{
public:
	virtual ~AsyncContext() {}

	HRESULT triggerEvent(E* event) { return m_stateMachine->triggerEvent(this, event); }
	S* getCurrentState() const { return (S*)m_currentState.p; }

	// Returns AsyncData object.
	virtual AsyncData* _getAsyncData() { return &m_asyncData; }

protected:
	AsyncData m_asyncData;
};

}
