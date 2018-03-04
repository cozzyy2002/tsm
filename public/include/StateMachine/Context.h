#pragma once

#include "Interface.h"

#include <memory>
namespace tsm {

template<class E = IEvent, class S = IState>
class Context : public IContext
{
public:
	virtual ~Context() {}

	virtual bool isAsync() const { return false; }

	HRESULT setup(S* initialState, E* event = nullptr) { return _getStateMachine()->setup(this, initialState, event); }
	HRESULT shutdown(DWORD timeout = 100) { return _getStateMachine()->shutdown(this, timeout); }
	HRESULT triggerEvent(E* event) { return _getStateMachine()->triggerEvent(this, event); }
	HRESULT handleEvent(E* event) { return _getStateMachine()->handleEvent(this, event); }
	HRESULT waitReady(DWORD timeout = 100) { return _getStateMachine()->waitReady(this, timeout); }
	S* getCurrentState() const { return (S*)m_currentState.p; }

	IStateMachine* _getStateMachine() {
		if(!m_stateMachine) m_stateMachine.reset(IStateMachine::create(this));
		return m_stateMachine.get();
	}
	virtual IState* _getCurrentState() { return m_currentState; }
	virtual void _setCurrentState(IState* state) { m_currentState = state; }

	// Returns nullptr(Async operation is not supported).
	virtual AsyncData* _getAsyncData() { return nullptr; }

	virtual lock_t* _getHandleEventLock() { return new lock_t(m_handleEventLock); }

	virtual IStateMonitor* _getStateMonitor() { return nullptr; }

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

	virtual bool isAsync() const { return true; }

	S* getCurrentState() const { return (S*)m_currentState.p; }

	// Returns AsyncData object.
	virtual AsyncData* _getAsyncData() { return &m_asyncData; }

protected:
	AsyncData m_asyncData;
};

}
