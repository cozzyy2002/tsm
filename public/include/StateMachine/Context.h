#pragma once

#include "Interface.h"
#include "TimerClient.h"

#include <memory>

namespace tsm {

template<class E = IEvent, class S = IState>
class Context : public IContext, public TimerClient
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

	virtual IStateMachine* _getStateMachine() override {
		if(!m_stateMachine) m_stateMachine.reset(IStateMachine::create(this));
		return m_stateMachine.get();
	}
	virtual IState* _getCurrentState() override { return m_currentState; }
	virtual void _setCurrentState(IState* state) override { m_currentState = state; }

	virtual lock_t* _getHandleEventLock() override { return new lock_t(m_handleEventLock); }

	virtual IStateMonitor* _getStateMonitor() override { return nullptr; }

	// Implementation of IContext::_getTimerClient().
	virtual TimerClient* _getTimerClient() override { return this; }

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
};

}
