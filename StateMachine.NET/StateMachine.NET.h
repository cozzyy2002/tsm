#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace Runtime::InteropServices;

namespace native
{
class Context;
class State;
class Event;
class StateMonitor;
}

namespace tsm_NET
{
// Define HResult in tsm_NET namespace
#include "HResult.h"

ref class Context;
ref class State;
ref class Event;

public interface class IStateMonitor
{
	void onIdle(Context^ context);
	void onEventTriggered(Context^ context, Event^ event);
	void onEventHandling(Context^ context, Event^ event, State^ current);
	void onStateChanged(Context^ context, Event^ event, State^ previous, State^ next);
	void onTimerStarted(Context^ context, Event^ event) = 0;
	void onTimerStopped(Context^ context, Event^ event, HResult hr) = 0;
	void onWorkerThreadExit(Context^ context, HResult exitCode) = 0;

	generic<typename H>
	ref class AssertFailedEventArgs : public EventArgs
	{
	public:
		AssertFailedEventArgs(H hr, String^ expression, String^ sourceFile, int lineNumber)
			: _hr(hr), _expression(expression), _sourceFile(sourceFile), _lineNumber(lineNumber) {}

		property H hr { H get() { return _hr; } }
		property String^ expression { String^ get() { return _expression; } }
		property String^ sourceFile { String^ get() { return _sourceFile; } }
		property int lineNumber { int get() { return _lineNumber; } }

	protected:
		H _hr;
		String^ _expression;
		String^ _sourceFile;
		int _lineNumber;
	};

	///*static*/ event EventHandler<AssertFailedEventArgs<HResult>^>^ AssertFailedEvent;
};

public ref class StateMonitorCaller
{
internal:
	StateMonitorCaller(IStateMonitor^ stateMonitor);
	virtual ~StateMonitorCaller();
	!StateMonitorCaller();

	using NativeType = native::StateMonitor;

#pragma region Definition of delegate, callback signature and callback method. See native::Callback<> template class.
	// void IStateMonitor::onIdle()
	virtual void onIdleCallback(tsm::IContext* context);
	// void IstateMonitor::onEventTriggered()
	virtual void onEventTriggeredCallback(tsm::IContext* context, tsm::IEvent* event);
	// void IStateMonitor::onEventHandling()
	virtual void onEventHandlingCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current);
	// void IStateMonitor::onStateChanged()
	virtual void onStateChangedCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next);
	// void IStateMonitor::onTimerStarted()
	virtual void onTimerStartedCallback(tsm::IContext* context, tsm::IEvent* event);
	// void IStateMonitor::onTimerStopped()
	virtual void onTimerStoppedCallback(tsm::IContext* context, tsm::IEvent* event, HRESULT hr);
	// void IStateMonitor::onWorkerThreadExit()
	virtual void onWorkerThreadExitCallback(tsm::IContext* context, HRESULT exitCode);
#pragma endregion

	NativeType* get() { return m_nativeStateMonitor; }

protected:
	NativeType* m_nativeStateMonitor;
	IStateMonitor^ m_stateMonitor;
};

public ref class TimerClient abstract
{
public:
	property IList<Event^>^ PendingEvents { IList<Event^>^ get(); }

internal:
	virtual tsm::TimerClient* getTimerClient() = 0;
};

public ref class Context : public TimerClient
{
	void construct(bool isAsync, bool useNativeThread);

protected:
	Context(bool isAsync, bool useNativeThread) { construct(isAsync, useNativeThread); }

public:
	Context() { construct(false, false); }
	virtual ~Context();
	!Context();

	static property unsigned int CurrentTherad { unsigned int get() { return GetCurrentThreadId(); }}

	bool isAsync();
	HResult setup(State^ initialState, Event^ event);
	HResult setup(State^ initialState) { return setup(initialState, nullptr); }
	HResult shutdown(TimeSpan timeout);
	HResult shutdonw(int timeout_msec);
	HResult shutdown() { return shutdown(TimeSpan::FromMilliseconds(100)); }
	HResult triggerEvent(Event^ event);
	HResult handleEvent(Event^ event);
	HResult waitReady(TimeSpan timeout);
	HResult waitReady(int timeout_msec);
	State^ getCurrentState();
	virtual HResult getAsyncExitCode([Out] HResult% hrExitCode) { return HResult::NotImpl; }

#pragma region .NET properties
	property IStateMonitor^ StateMonitor {
		IStateMonitor^ get() { return m_stateMonitor; }
		void set(IStateMonitor^ value);
	}

	property bool IsAsync { bool get() { return isAsync(); } }
	property State^ CurrentState { State^ get() { return getCurrentState(); } }
#pragma endregion

internal:
	using NativeType = native::Context;

	NativeType* get() { return m_nativeContext; }
	property bool useNativeThread { bool get() { return m_useNativeThread; } }

protected:
	NativeType* m_nativeContext;
	bool m_useNativeThread;
	StateMonitorCaller^ m_stateMonitorCaller;
	tsm_NET::IStateMonitor^ m_stateMonitor;

internal:
	virtual tsm::TimerClient* getTimerClient() override;
};

public ref class AsyncContext : public Context
{
public:
	AsyncContext() : Context(true, false) {}
	AsyncContext(bool useNativeThread) : Context(true, useNativeThread) {}
	HResult getAsyncExitCode([Out] HResult% hrExitCode) override;
};

extern HRESULT getAsyncExitCode(native::Context* context, HRESULT* phr);

public ref class State : public TimerClient
{
	void construct(State^ masterState, bool autoDispose);

public:
	State() { construct(nullptr, DefaultAutoDispose); }
	State(bool autoDispose) { construct(nullptr, autoDispose); }
	State(State^ masterState) { construct(masterState, masterState ? masterState->AutoDispose : DefaultAutoDispose); }
	State(State^ masterState, bool autoDispose) { construct(masterState, autoDispose); }
	~State();
	!State();

#pragma region Methods to be implemented by sub class.
	virtual HResult handleEvent(Context^ context, Event^ event, State^% nextState) { return HResult::Ok; }
	virtual HResult entry(Context^ context, Event^ event, State^ previousState) { return HResult::Ok; }
	virtual HResult exit(Context^ context, Event^ event, State^ nextState) { return HResult::Ok; }
#pragma endregion

	State^ getMasterState();
	//State^ getSubState();
	bool isSubState();

#pragma region .NET properties
	property State^ MasterState { State^ get() { return getMasterState(); } }
	property bool IsSubState { bool get() { return isSubState(); } }
	property bool IsExitCalledOnShutdown;

	property long SequenceNumber { long get(); }
	static property int MemoryWeight { int get(); void set(int value); }

	/**
	 * AutoDispose : Determine the way to dispose this object.
	 * If this value is true(default)
	 *	* State object is disposed after State::exit() is called.
	 * else
	 *	* StateMachine never disposes State object.
	 *	* The object should be disposed by user.
	 *
	 * This value is specified by autoDispose argument of constructor.
	 */
	property bool AutoDispose { bool get(); }

	static bool DefaultAutoDispose = true;
#pragma endregion

internal:
	using NativeType = native::State;

	NativeType* get() { return m_nativeState; }

protected:
	NativeType* m_nativeState;

internal:
	virtual tsm::TimerClient* getTimerClient() override;
};

public ref class Event
{
	void construct(int priority, bool autoDispose);

public:
	Event() { construct(0, DefaultAutoDispose); }
	Event(bool autoDispose) { construct(0, autoDispose); }
	Event(int priority) { construct(priority, DefaultAutoDispose); }
	Event(int priority, bool autoDispose) { construct(priority, autoDispose); }
	~Event();
	!Event();

#pragma region Methods to be implemented by sub class.
	virtual HResult preHandle(Context^ context) { return HResult::Ok; }
	virtual HResult postHandle(Context^ context, HResult hr) { return hr; }
#pragma endregion

	void setDelayTimer(TimerClient^ client, TimeSpan delayTime);
	void setIntervalTimer(TimerClient^ client, TimeSpan intervalTime);
	void setTimer(TimerClient^ client, TimeSpan delayTime, TimeSpan intervalTime);
	void setDelayTimer(TimerClient^ client, int delayTime_msec);
	void setIntervalTimer(TimerClient^ client, int intervalTime_msec);
	void setTimer(TimerClient^ client, int delayTime_msec, int intervalTime_msec);
	HResult cancelTimer() { return cancelTimer(0); }
	HResult cancelTimer(TimeSpan timeout) { return cancelTimer((int)timeout.TotalMilliseconds); }
	HResult cancelTimer(int timeout);
	property TimeSpan DelayTime { TimeSpan get(); }
	property TimeSpan InterValTime { TimeSpan get(); }
	property int TimeoutCount { int get(); }
	property int Priority { int get(); }

#pragma region .NET properties
	property long SequenceNumber { long get(); }
	static property int MemoryWeight { int get(); void set(int value); }

	/**
	 * AutoDispose : Determine the way to dispose this object.
	 * If this value is true(default)
	 *	* Event object is disposed when Context::handleEvent(Event) is completed.
	 * else
	 *	* StateMachine never disposes Event object.
	 *	* The object should be disposed by user.
	 *  * Then user can use one Event object to call Context::handleEvent(Event) more than once.
	 *
	 * This value is specified by autoDispose argument of constructor.
	 */
	property bool AutoDispose { bool get(); }

	static bool DefaultAutoDispose = true;
#pragma endregion

internal:
	using NativeType = native::Event;

	NativeType* get() { return m_nativeEvent; }

protected:
	NativeType* m_nativeEvent;

	void setTimer(tsm::TimerClient* timerClient, int delayTime, int intervalTime);
};

public ref class Error
{
public:
	using UINT32 = HRESULT;
	Error(HRESULT hr);
	Error(HResult hr);
	property HRESULT HResult { HRESULT get(); }
	property String^ Message { String^ get(); }

protected:
	HRESULT m_hr;
	String^ m_message;
};

namespace common
{
template<class C>
typename C::NativeType* getNative(C^ managed)
{
	return managed ? managed->get() : nullptr;
}

template<class C>
typename C::ManagedType^ getManaged(C* native)
{
	return native ? native->get() : nullptr;
}

}

}
