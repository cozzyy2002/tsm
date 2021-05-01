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
	virtual HRESULT getAsyncExitCode(HRESULT* pht) { return E_NOTIMPL; }
	virtual IAsyncDispatcher* _createAsyncDispatcher() override { return nullptr; }

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

	virtual IStateMonitor* _getStateMonitor() override { return nullptr; }

	// Implementation of IContext::_getTimerClient().
	virtual TimerClient* _getTimerClient() override { return this; }

	virtual ContextHandle* _getHandle(bool reset = false) override {
		if(!m_handle || reset) {
			// NOTE: Creating ContextHandle object refers IContext::isAsync().
			//       So it should be performed after constructing Context object including derived class.
			m_handle.reset(HandleFactory<IContext, ContextHandle>::create(this));
		}
		return m_handle.get();
	}

	S* getCurrentState() { return (S*)_getCurrentState(); }

protected:
	std::unique_ptr<ContextHandle, HandleFactory<IContext, ContextHandle>> m_handle;
	std::unique_ptr<IStateMachine> m_stateMachine;
	CComPtr<IState> m_currentState;
};

tsm_STATE_MACHINE_EXPORT HRESULT Context_getAsyncExitCode(IContext* context, HRESULT* phr);
tsm_STATE_MACHINE_EXPORT IAsyncDispatcher* Context_createAsyncDispatcher();

template<class E = IEvent, class S = IState>
class AsyncContext : public Context<E, S>
{
public:
	virtual ~AsyncContext() {}

	virtual bool isAsync() const { return true; }
	virtual HRESULT getAsyncExitCode(HRESULT* phr) override { return Context_getAsyncExitCode(this, phr); }
	virtual IAsyncDispatcher* _createAsyncDispatcher() override { return Context_createAsyncDispatcher(); }
};

}
