#pragma once

using namespace System;
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
	// void IStateMonitor::onWorkerThreadExit()
	virtual void onWorkerThreadExitCallback(tsm::IContext* context, HRESULT exitCode);
#pragma endregion

	NativeType* get() { return m_nativeStateMonitor; }

protected:
	NativeType* m_nativeStateMonitor;
	IStateMonitor^ m_stateMonitor;
};

public ref class Context
{
	void construct(bool isAsync, bool useNativeThread);

protected:
	Context(bool isAsync, bool useNativeThread) { construct(isAsync, useNativeThread); }

public:
	Context() { construct(false, false); }
	virtual ~Context();
	!Context();

	bool isAsync();
	virtual HResult getAsyncExitCode([Out] HResult% hrExitCode) { return (HResult)E_NOTIMPL; }
	HResult setup(State^ initialState, Event^ event);
	HResult setup(State^ initialState) { return setup(initialState, nullptr); }
	HResult shutdown(TimeSpan timeout);
	HResult shutdown() { return shutdown(TimeSpan::FromMilliseconds(100)); }
	HResult triggerEvent(Event^ event);
	HResult handleEvent(Event^ event);
	HResult waitReady(TimeSpan timeout);
	State^ getCurrentState();

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
};

public ref class AsyncContext : public Context
{
public:
	AsyncContext() : Context(true, false) {}
	AsyncContext(bool useNativeThread) : Context(true, useNativeThread) {}
	virtual HResult getAsyncExitCode([Out] HResult% hrExitCode) override;

protected:
};

extern HRESULT getAsyncExitCode(native::Context* context, HRESULT* phr);

public ref class State
{
public:
	State() : State(nullptr) {}
	State(State^ masterState);

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
#pragma endregion

internal:
	using NativeType = native::State;

	NativeType* get() { return m_nativeState; }

protected:
	NativeType* m_nativeState;
};

public ref class Event
{
public:
	Event();

#pragma region Methods to be implemented by sub class.
	virtual HResult preHandle(Context^ context) { return HResult::Ok; }
	virtual HResult postHandle(Context^ context, HResult hr) { return hr; }
#pragma endregion

	//
	// TODO: Implement setTimer() method.
	//

#pragma region .NET properties
	property long SequenceNumber { long get(); }
	static property int MemoryWeight { int get(); void set(int value); }
#pragma endregion

internal:
	using NativeType = native::Event;

	NativeType* get() { return m_nativeEvent; }

protected:
	NativeType* m_nativeEvent;
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
