#pragma once

#include "Interface.h"

#include <memory>
namespace tsm {

template<class E = IEvent, class S = IState>
class Context : public IContext
{
public:
	Context() { m_stateMachine.reset(createStateMachine()); }
	virtual ~Context() {}

	HRESULT setup(S* initialState, E* event = nullptr) { return m_stateMachine->setup(this, initialState, event); }
	HRESULT shutdown() { return m_stateMachine->shutdown(this); }
	HRESULT handleEvent(E* event) { return m_stateMachine->handleEvent(this, event); }
	HRESULT waitReady(DWORD timeout = 100) { return m_stateMachine->waitReady(this, timeout); }

	// Returns nullptr(Async operation is not supported).
	virtual AsyncData* getAsyncData() { return nullptr; }

	S* getCurrentState() const { return (S*)m_currentState.p; }
};

template<class E = IEvent, class S = IState>
class AsyncContext : public Context<E, S>
{
public:
	virtual ~AsyncContext() {}

	HRESULT triggerEvent(E* event) { return m_stateMachine->triggerEvent(this, event); }

	// Returns AsyncData object.
	virtual AsyncData* getAsyncData() { return &m_asyncData; }

protected:
	AsyncData m_asyncData;
};

}
