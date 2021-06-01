#pragma once

#include "StateMachine.NET.h"

namespace tsm_NET
{
namespace Generic
{
using HResult = tsm_NET::HResult;

generic<typename E, typename S>
	where E : tsm_NET::IEvent
	where S : tsm_NET::IState
ref class Context;
generic<typename C, typename E, typename S>
	where C : tsm_NET::IContext
	where E : tsm_NET::IEvent
	where S : tsm_NET::IState
ref class State;
generic<typename C>
	where C : tsm_NET::IContext
ref class Event;

generic<typename E, typename S>
	where E : tsm_NET::IEvent
	where S : tsm_NET::IState
public interface class IStateMonitor
{
	void onIdle(Context<E, S>^ context);
	void onEventTriggered(Context<E, S>^ context, E event);
	void onEventHandling(Context<E, S>^ context, E event, S current);
	void onStateChanged(Context<E, S>^ context, E event, S previous, S next);
	void onTimerStarted(Context<E, S>^ context, E event);
	void onTimerStopped(Context<E, S>^ context, E event, HResult hr);
	void onWorkerThreadExit(Context<E, S>^ context, HResult exitCode);
};

generic<typename E, typename S>
	where E : tsm_NET::IEvent
	where S : tsm_NET::IState
public ref class StateMonitorCaller : public tsm_NET::StateMonitorCaller
{
internal:
	StateMonitorCaller(IStateMonitor<E, S>^ stateMonitor);

	virtual void onIdleCallback(tsm::IContext* context) override;
	virtual void onEventTriggeredCallback(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onEventHandlingCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current) override;
	virtual void onStateChangedCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next) override;
	virtual void onTimerStartedCallback(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onTimerStoppedCallback(tsm::IContext* context, tsm::IEvent* event, HRESULT hr) override;
	virtual void onWorkerThreadExitCallback(tsm::IContext* context, HRESULT exitCode) override;

protected:
	IStateMonitor<E, S>^ m_stateMonitor;
};

generic<typename E, typename S>
	where E : tsm_NET::IEvent
	where S : tsm_NET::IState
public ref class Context : public tsm_NET::IContext
{
private:
	void construct(bool isAsync, bool useNativeThread);

protected:
	// Protected constructor called by AsyncContext derived class.
	Context(bool isAsync, bool useNativeThread) : m_stateMonitor(nullptr) { construct(isAsync, useNativeThread); }

public:
	Context() : m_stateMonitor(nullptr) { construct(false, false); }
	virtual ~Context() {}

	HResult setup(S initialState, E event);
	HResult setup(S initialState);
	HResult shutdown(TimeSpan timeout);
	HResult shutdown(int timeout_msec);
	HResult shutdown();
	HResult triggerEvent(E event);
	HResult handleEvent(E event);
	HResult waitReady(TimeSpan timeout);
	HResult waitReady(int timeout_msec);
	S getCurrentState();
	virtual HResult getAsyncExitCode([Out] HResult% hrExitCode) { return HResult::NotImpl; }

	property S CurrentState { S get() { return getCurrentState(); } }

#pragma region Implementation of IContext
public:
	IContext::NativeType* get() { return m_nativeContext; }
	virtual property bool UseNativeThread { bool get() { return m_useNativeThread; } }

	virtual property IList<IEvent^>^ PendingEvents { IList<IEvent^>^ get(); }
	virtual tsm::ITimerClient* getTimerClient();
#pragma endregion

#pragma region .NET properties
	property IStateMonitor<E, S>^ StateMonitor
	{
		IStateMonitor<E, S>^ get() { return m_stateMonitor; }
		void set(IStateMonitor<E, S>^ value);
	}
#pragma endregion


protected:
	IContext::NativeType* m_nativeContext;
	bool m_useNativeThread;
	IStateMonitor<E, S>^ m_stateMonitor;
	StateMonitorCaller<E, S>^ m_stateMonitorCaller;
};

generic<typename E, typename S>
	where E : tsm_NET::IEvent
	where S : tsm_NET::IState
public ref class AsyncContext : public Context<E, S>
{
public:
	AsyncContext() : Context(true, false) {}
	AsyncContext(bool useNativeThread) : Context(true, useNativeThread) {}

	HResult getAsyncExitCode([Out] HResult% hrExitCode) override;
};

generic<typename C, typename E, typename S>
	where C : tsm_NET::IContext
	where E : tsm_NET::IEvent
	where S : tsm_NET::IState
public ref class State : public tsm_NET::IState, public tsm_NET::IAutoDisposable
{
	void construct(S masterState, bool autoDispose);

public:
	State() { construct(S(), DefaultAutoDispose); }
	State(bool autoDispose) { construct(S(), autoDispose); }
	State(S masterState) { construct(masterState, DefaultAutoDispose); }
	State(S masterState, bool autoDispose) { construct(masterState, autoDispose); }

	~State();
	!State();

#pragma region Methods to be implemented by sub class.
	virtual HResult handleEvent(C context, E event, S% nextState) { return HResult::Ok; }
	virtual HResult entry(C context, E event, S previousState) { return HResult::Ok; }
	virtual HResult exit(C context, E event, S nextState) { return HResult::Ok; }
#pragma endregion

	S getMasterState();

	property S MasterState { S get() { return getMasterState(); } }

#pragma region Override methods of tsm_NET::IState that call sub class with generic parameters.
	virtual HResult _handleEvent(tsm_NET::IContext^ context, tsm_NET::IEvent^ event, tsm_NET::IState^% nextState) sealed;
	virtual HResult _entry(tsm_NET::IContext^ context, tsm_NET::IEvent^ event, tsm_NET::IState^ previousState) sealed;
	virtual HResult _exit(tsm_NET::IContext^ context, tsm_NET::IEvent^ event, tsm_NET::IState^ nextState) sealed;

	// IsExitCallOnShutdown property returns false as default.
	// Sub class may override if necessary
	virtual property bool IsExitCalledOnShutdown { bool get() { return false; } }

	virtual property bool AutoDispose { bool get() sealed; }
	static bool DefaultAutoDispose = true;

	property long SequenceNumber { long get(); }
	static property int MemoryWeight { int get(); void set(int value); }

	virtual IState::NativeType* get() sealed { return m_nativeState; }

	virtual property IList<IEvent^>^ PendingEvents { IList<IEvent^>^ get(); }
	virtual tsm::ITimerClient* getTimerClient();
#pragma endregion

internal:
	IState::NativeType* m_nativeState;
};

generic<typename C>
	where C : tsm_NET::IContext
public ref class Event : public tsm_NET::IEvent, public tsm_NET::IAutoDisposable
{
	void construct(int priority, bool autoDispose);

public:
	Event() { construct(0, DefaultAutoDispose); }
	Event(bool autoDispose) { construct(0, autoDispose); }
	Event(int priority) { construct(priority, DefaultAutoDispose); }
	Event(int priority, bool autoDispose) { construct(priority, DefaultAutoDispose); }

	~Event();
	!Event();

	void setTimer(tsm_NET::ITimerOwner^ timerOwner, TimeSpan delayTime, TimeSpan intervalTime) { setTimer(timerOwner, (int)delayTime.TotalMilliseconds, (int)intervalTime.TotalMilliseconds); }
	void setTimer(tsm_NET::ITimerOwner^ timerOwner, int delayTime_msec, int intervalTime_msec);
	void setDelayTime(tsm_NET::ITimerOwner^ timerOwner, TimeSpan delayTime) { setTimer(timerOwner, (int)delayTime.TotalMilliseconds, 0); }
	void setDelayTime(tsm_NET::ITimerOwner^ timerOwner, int delayTime_msec) { setTimer(timerOwner, delayTime_msec, 0); }
	void setIntervalTime(tsm_NET::ITimerOwner^ timerOwner, TimeSpan intervalTime) { setTimer(timerOwner, 0, (int)intervalTime.TotalMilliseconds); }
	void setIntervalTime(tsm_NET::ITimerOwner^ timerOwner, int intervalTime_msec) { setTimer(timerOwner, 0, intervalTime_msec); }

	property TimeSpan DelayTime { TimeSpan get(); }
	property TimeSpan IntervalTime { TimeSpan get(); }
	property int TimeoutCount { int get(); }
	property int Priority { int get(); }

	HResult cancelTimer();
	HResult cancelTimer(TimeSpan timeout);
	HResult cancelTimer(int timeout);

#pragma region Methods to be implemented by sub class.
	virtual HResult preHandle(C context) { return HResult::Ok; }
	virtual HResult postHandle(C context, HResult hr) { return hr; }
#pragma endregion

#pragma region Methods that call sub class with generic parameters.
	virtual HResult _preHandle(tsm_NET::IContext^ context) sealed;
	virtual HResult _postHandle(tsm_NET::IContext^ context, HResult hr) sealed;
#pragma endregion

	virtual property bool AutoDispose { bool get() sealed; }
	static bool DefaultAutoDispose = true;

	property long SequenceNumber { long get(); }
	static property int MemoryWeight { int get(); void set(int value); }

	virtual IEvent::NativeType* get() sealed { return m_nativeEvent; }

protected:
	IEvent::NativeType* m_nativeEvent;
};

public ref class Error : public tsm_NET::Error
{
public:
	Error(HRESULT hr) : tsm_NET::Error(hr) {}
	Error(tsm_NET::Generic::HResult hr) : tsm_NET::Error((HRESULT)hr) {}
};

}
}
