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

class Context : public tsm::Context<Event, State>
{
public:
	using ManagedType = tsm_NET::IContext;

	Context(ManagedType^ context, bool isAsync = true);
	virtual bool isAsync() const override { return m_isAsync; }
	virtual HRESULT getAsyncExitCode(HRESULT* pht) override;
	virtual tsm::IAsyncDispatcher* _createAsyncDispatcher() override;

	virtual tsm::IStateMonitor* _getStateMonitor() override { return m_stateMonitor; }
	virtual tsm::ContextHandle* _getHandle(bool reset = false) override { return tsm::Context<Event, State>::_getHandle(reset); }
	void setStateMonitor(tsm::IStateMonitor* stateMonitor) { m_stateMonitor = stateMonitor; }

	ManagedType^ get() { return m_managedContext; }

	inline State* getCurrentState() { return (State*)_getCurrentState(); }

protected:
	gcroot<ManagedType^> m_managedContext;
	bool m_isAsync;

	tsm::IStateMonitor* m_stateMonitor;
};

class State : public tsm::State<Context, Event, State>
{
public:
	using ManagedType = tsm_NET::IState;

	State(ManagedType^ state, ManagedType^ masterState, bool autoDispose);
	virtual ~State();

#pragma region Implementation of IState that call methods of managed class.
	virtual HRESULT handleEvent(Context* context, Event* event, State** nextState) override;
	virtual HRESULT entry(Context* context, Event* event, State* previousState) override;
	virtual HRESULT exit(Context* context, Event* event, State* nextState) override;

	virtual bool _isExitCalledOnShutdown() const override;
#pragma endregion

	ManagedType^ get() { return m_managedState; }

	const bool m_autoDispose;

protected:
	gcroot<ManagedType^> m_managedState;
};

class Event : public tsm::Event<Context>
{
public:
	using ManagedType = tsm_NET::IEvent;

	Event(ManagedType^ event, int priority, bool autoDispose);
	virtual ~Event();

#pragma region Implementation of IState that call methods of managed class.
	virtual HRESULT preHandle(Context* context) override;
	virtual HRESULT postHandle(Context* context, HRESULT hr) override;
#pragma endregion

	ManagedType^ get() { return m_managedEvent; }

	const bool m_autoDispose;

protected:
	gcroot<ManagedType^> m_managedEvent;
};
}