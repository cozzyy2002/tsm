#pragma once

#include "StateMachine.NET.h"

namespace tsm_NET
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

generic<typename C, typename E, typename S>
	where C : tsm_NET::IContext
	where E : tsm_NET::IEvent
	where S : tsm_NET::IState
public ref class StateMonitor : public IStateMonitor
{
public:
	StateMonitor();
	~StateMonitor();
	!StateMonitor();

	virtual void onIdle(IContext^ context) sealed { onIdle((C)context); }
	virtual void onEventTriggered(IContext^ context, IEvent^ event) sealed { onEventTriggered((C)context, (E)event); }
	virtual void onEventHandling(IContext^ context, IEvent^ event, IState^ current) sealed { onEventHandling((C)context, (E)event, (S)current); }
	virtual void onStateChanged(IContext^ context, IEvent^ event, IState^ previous, IState^ next) sealed { onStateChanged((C)context, (E)event, (S)previous, (S)next); }
	virtual void onTimerStarted(IContext^ context, IEvent^ event) sealed { onTimerStarted((C)context, (E)event); }
	virtual void onTimerStopped(IContext^ context, IEvent^ event, HResult hr) sealed { onTimerStopped((C)context, (E)event, hr); }
	virtual void onWorkerThreadExit(IContext^ context, HResult exitCode) sealed { onWorkerThreadExit((C)context, exitCode); }
	virtual IStateMonitor::NativeType* get() sealed { return m_nativeStateMonitor; }

#pragma region Methods to be implemented by sub class.
	virtual void onIdle(C context) {}
	virtual void onEventTriggered(C context, E event) {}
	virtual void onEventHandling(C context, E event, S current) {}
	virtual void onStateChanged(C context, E event, S previous, S next) {}
	virtual void onTimerStarted(C context, E event) {}
	virtual void onTimerStopped(C context, E event, HResult hr) {}
	virtual void onWorkerThreadExit(C context, HResult exitCode) {}
#pragma endregion

protected:
	IStateMonitor::NativeType* m_nativeStateMonitor;
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
	~Context();
	!Context();

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

	property bool IsAsync { bool get(); }
	property S CurrentState { S get() { return getCurrentState(); } }

#pragma region Implementation of IContext
public:
	IContext::NativeType* get() { return m_nativeContext; }
	virtual property bool UseNativeThread { bool get() { return m_useNativeThread; } }

	virtual property IList<IEvent^>^ PendingEvents { IList<IEvent^>^ get() sealed; }
	virtual tsm::ITimerClient* getTimerClient() sealed;
#pragma endregion

#pragma region .NET properties
	property IStateMonitor^ StateMonitor
	{
		IStateMonitor^ get() { return m_stateMonitor; }
		void set(IStateMonitor^ value);
	}

	static property unsigned int CurrentThread { unsigned int get() { return GetCurrentThreadId(); }}
#pragma endregion


protected:
	IContext::NativeType* m_nativeContext;
	bool m_useNativeThread;
	IStateMonitor^ m_stateMonitor;
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

	virtual ~State();
	!State();

#pragma region Methods to be implemented by sub class.
	virtual HResult handleEvent(C context, E event, S% nextState) { return HResult::Ok; }
	virtual HResult entry(C context, E event, S previousState) { return HResult::Ok; }
	virtual HResult exit(C context, E event, S nextState) { return HResult::Ok; }
#pragma endregion

	property bool IsSubState { bool get(); }
	S getMasterState();

	property S MasterState { S get() { return getMasterState(); } }

#pragma region Override methods of tsm_NET::IState that call sub class with generic parameters.
	virtual HResult _handleEvent(tsm_NET::IContext^ context, tsm_NET::IEvent^ event, tsm_NET::IState^% nextState) sealed;
	virtual HResult _entry(tsm_NET::IContext^ context, tsm_NET::IEvent^ event, tsm_NET::IState^ previousState) sealed;
	virtual HResult _exit(tsm_NET::IContext^ context, tsm_NET::IEvent^ event, tsm_NET::IState^ nextState) sealed;

	// IsExitCallOnShutdown property returns false as default.
	// Sub class may override if necessary
	virtual property bool IsExitCalledOnShutdown;

	virtual property bool AutoDispose { bool get() sealed; }
	static bool DefaultAutoDispose = true;

	property long SequenceNumber { long get(); }
	static property int MemoryWeight { int get(); void set(int value); }

	virtual IState::NativeType* get() sealed { return m_nativeState; }

	virtual property IList<IEvent^>^ PendingEvents { IList<IEvent^>^ get() sealed; }
	virtual tsm::ITimerClient* getTimerClient() sealed;
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

}
