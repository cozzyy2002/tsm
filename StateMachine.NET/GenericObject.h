#pragma once

#include "StateMachine.NET.h"

namespace tsm_NET
{
namespace Generic
{
// Define HResult in tsm_NET::Getneric namespace
#include "HResult.h"

generic<typename E, typename S>
ref class Context;

generic<typename E, typename S>
public interface class IStateMonitor
{
	void onIdle(Context<E, S>^ context);
	void onEventTriggered(Context<E, S>^ context, E event);
	void onEventHandling(Context<E, S>^ context, E event, S current);
	void onStateChanged(Context<E, S>^ context, E event, S previous, S next);
	void onTimerStarted(Context<E, S>^ context, E event);
	void onWorkerThreadExit(Context<E, S>^ context, HResult exitCode);
};

generic<typename E, typename S>
public ref class StateMonitorCaller : public tsm_NET::StateMonitorCaller
{
internal:
	StateMonitorCaller(IStateMonitor<E, S>^ stateMonitor);

	virtual void onIdleCallback(tsm::IContext* context) override;
	virtual void onEventTriggeredCallback(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onEventHandlingCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current) override;
	virtual void onStateChangedCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next) override;
	virtual void onTimerStartedCallback(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onWorkerThreadExitCallback(tsm::IContext* context, HRESULT exitCode) override;

protected:
	IStateMonitor<E, S>^ m_stateMonitor;
};

generic<typename E, typename S>
public ref class Context : public tsm_NET::Context
{
protected:
	Context(bool isAsync, bool useNativeThread) : tsm_NET::Context(isAsync, useNativeThread), m_stateMonitor(nullptr) {}

public:
	Context() : tsm_NET::Context(false, false), m_stateMonitor(nullptr) {}
	virtual ~Context() {}

	HResult setup(S initialState, E event) { return (HResult)tsm_NET::Context::setup((tsm_NET::State^)initialState, (tsm_NET::Event^)event); }
	HResult setup(S initialState) { return (HResult)tsm_NET::Context::setup((tsm_NET::State^)initialState); }
	HResult shutdown(TimeSpan timeout) { return (HResult)tsm_NET::Context::shutdown(timeout); }
	HResult shutdown() { return (HResult)tsm_NET::Context::shutdown(); }
	HResult triggerEvent(E event) { return (HResult)tsm_NET::Context::triggerEvent((tsm_NET::Event^)event); }
	HResult handleEvent(E event) { return (HResult)tsm_NET::Context::handleEvent((tsm_NET::Event^)event); }
	HResult waitReady(TimeSpan timeout) { return (HResult)tsm_NET::Context::waitReady(timeout); }
	S getCurrentState() { return (S)tsm_NET::Context::getCurrentState(); }

	property S CurrentState { S get() { return getCurrentState(); } }

#pragma region .NET properties
	property IStateMonitor<E, S>^ StateMonitor
	{
		IStateMonitor<E, S>^ get() { return m_stateMonitor; }
		void set(IStateMonitor<E, S>^ value);
	}
#pragma endregion

protected:
	IStateMonitor<E, S>^ m_stateMonitor;
	StateMonitorCaller<E, S>^ m_stateMonitorCaller;
};

generic<typename E, typename S>
public ref class AsyncContext : public Context<E, S>
{
public:
	AsyncContext() : Context(true, false) {}
	AsyncContext(bool useNativeThread) : Context(true, useNativeThread) {}
};

generic<typename C, typename E, typename S>
	where C : tsm_NET::Context
	where E : tsm_NET::Event
	where S : tsm_NET::State
public ref class State : public tsm_NET::State
{
public:
	State() : tsm_NET::State(nullptr) {}
	State(S masterState) : tsm_NET::State((tsm_NET::State^)masterState) {}

#pragma region Methods to be implemented by sub class.
	virtual HResult handleEvent(C context, E event, S% nextState) { return HResult::Ok; }
	virtual HResult entry(C context, E event, S previousState) { return HResult::Ok; }
	virtual HResult exit(C context, E event, S nextState) { return HResult::Ok; }
#pragma endregion

	S getMasterState() { return (S)tsm_NET::State::getMasterState(); }

	property S MasterState { S get() { return getMasterState(); } }

#pragma region Override methods of tsm_NET::State that call sub class with generic parameters.
	virtual tsm_NET::HResult handleEvent(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^% nextState) override sealed;
	virtual tsm_NET::HResult entry(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^ previousState) override sealed;
	virtual tsm_NET::HResult exit(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^ nextState) override sealed;
#pragma endregion
};

generic<typename C>
	where C : tsm_NET::Context
public ref class Event : public tsm_NET::Event
{
public:

#pragma region Methods to be implemented by sub class.
	virtual HResult preHandle(C context) { return HResult::Ok; }
	virtual HResult postHandle(C context, HResult hr) { return hr; }
#pragma endregion

#pragma region Methods that call sub class with generic parameters.
	virtual tsm_NET::HResult preHandle(tsm_NET::Context^ context) override sealed;
	virtual tsm_NET::HResult postHandle(tsm_NET::Context^ context, tsm_NET::HResult hr) override sealed;
#pragma endregion
};
}
}
