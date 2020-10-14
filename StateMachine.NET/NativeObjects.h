#pragma once

#include "StateMachine.NET.h"

namespace native
{
class Context;
class State;
class Event;

class StateMonitor : public tsm::IStateMonitor
{
public:
	using OwnerType = tsm_NET::StateMonitorCaller;

	StateMonitor(OwnerType^ owner);

	virtual void onIdle(tsm::IContext* context) override;
	virtual void onEventTriggered(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current) override;

	/**
	 * When setup(), previous is nullptr.
	 * When shutdown(), next is nullptr.
	 */
	virtual void onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next) override;
	virtual void onTimerStarted(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onTimerStopped(tsm::IContext* context, tsm::IEvent* event, HRESULT hr) override;
	virtual void onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode) override;

protected:
	gcroot<OwnerType^> m_owner;
};

class Context : public tsm::IContext, public tsm::TimerClient
{
public:
	using ManagedType = tsm_NET::Context;

	Context(ManagedType^ context, bool isAsync = true);
	virtual bool isAsync() const override { return m_isAsync; }
	virtual HRESULT getAsyncExitCode(HRESULT* pht) override;
	virtual tsm::IAsyncDispatcher* _createAsyncDispatcher() override;

	HRESULT setup(tsm::IState* initialState, tsm::IEvent* event = nullptr) { return _getStateMachine()->setup(this, initialState, event); }
	HRESULT shutdown(DWORD timeout = 100) { return _getStateMachine()->shutdown(this, timeout); }
	HRESULT triggerEvent(tsm::IEvent* event) { return _getStateMachine()->triggerEvent(this, event); }
	HRESULT handleEvent(tsm::IEvent* event) { return _getStateMachine()->handleEvent(this, event); }
	HRESULT waitReady(DWORD timeout = 100) { return _getStateMachine()->waitReady(this, timeout); }

	virtual tsm::IStateMachine* _getStateMachine() override {
		if(!m_stateMachine) m_stateMachine.reset(tsm::IStateMachine::create(this));
		return m_stateMachine.get();
	}
	virtual tsm::IState* _getCurrentState() override { return m_currentState; }
	virtual void _setCurrentState(tsm::IState* state) override { m_currentState = state; }

	virtual tsm::IStateMonitor* _getStateMonitor() override { return m_stateMonitor; }
	void setStateMonitor(tsm::IStateMonitor* stateMonitor) { m_stateMonitor = stateMonitor; }

	// Implementation of IContext::_getTimerClient().
	virtual TimerClient* _getTimerClient() override { return this; }

	ManagedType^ get() { return m_managedContext; }

	inline State* getCurrentState() { return (State*)_getCurrentState(); }

protected:
	gcroot<ManagedType^> m_managedContext;
	bool m_isAsync;

	std::unique_ptr<tsm::IStateMachine> m_stateMachine;
	CComPtr<tsm::IState> m_currentState;

	tsm::IStateMonitor* m_stateMonitor;
};

class State : public tsm::IState, public tsm::TimerClient
{
public:
	using ManagedType = tsm_NET::State;

	State(ManagedType^ state, ManagedType^ masterState, bool autoDispose);
	virtual ~State();

#pragma region Implementation of IState that call methods of managed class.
	virtual HRESULT _handleEvent(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState) override;
	virtual HRESULT _entry(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState) override;
	virtual HRESULT _exit(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState) override;

	virtual bool _isExitCalledOnShutdown() const override;

	virtual IState* _getMasterState() const override { return m_masterState; }

	virtual TimerClient* _getTimerClient() override { return this; }
#pragma endregion

	inline State* getMasterState() const { return (State*)_getMasterState(); }
	bool isSubState() const { return m_masterState ? true : false; }

	ManagedType^ get() { return m_managedState; }

	const bool m_autoDispose;

protected:
	gcroot<ManagedType^> m_managedState;
	CComPtr<State> m_masterState;
};

class Event : public tsm::IEvent
{
public:
	using ManagedType = tsm_NET::Event;

	Event(ManagedType^ event, int priority, bool autoDispose);
	virtual ~Event();

#pragma region Implementation of IState that call methods of managed class.
	virtual HRESULT _preHandle(tsm::IContext* Icontext) override;
	virtual HRESULT _postHandle(tsm::IContext* Icontext, HRESULT hr) override;
#pragma endregion

	void setTimer(tsm::TimerClient* timerClient, DWORD delayTime, DWORD intervalTime = 0);

	virtual int _getPriority() const override { return m_priority; }
	virtual DWORD _getDelayTime() const override { return m_delayTime; }
	virtual DWORD _getIntervalTime() const override { return m_intervalTime; }
	virtual tsm::TimerClient* _getTimerClient() const override { return m_timerClient; }
	virtual int _getTimeoutCount() const override { return m_timeoutCount; }
protected:
	virtual void _setTimeoutCount(int count) override { m_timeoutCount = count; }
public:

	ManagedType^ get() { return m_managedEvent; }

	const bool m_autoDispose;

protected:
	gcroot<ManagedType^> m_managedEvent;

	int m_priority;
	DWORD m_delayTime;
	DWORD m_intervalTime;
	tsm::TimerClient* m_timerClient;
	int m_timeoutCount;
};
}