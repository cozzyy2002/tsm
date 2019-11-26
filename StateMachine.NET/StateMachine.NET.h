#pragma once

using namespace System;

namespace native
{
class Context;
class State;
class Event;
class StateMonitor;
}

namespace tsm_NET
{
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

public:
	using NativeType = native::StateMonitor;

internal:
#pragma region Definition of delegate, callback signature and callback method. See native::Callback<> template class.
	// IStateMonitor::onIdle()
	delegate void OnIdleDelegate(tsm::IContext* context);
	typedef void(__stdcall *OnIdleCallback)(tsm::IContext* context);
	virtual void onIdleCallback(tsm::IContext* context);
	// IstateMonitor::onEventTriggered()
	delegate void OnEventTriggeredDelegate(tsm::IContext* context, tsm::IEvent* event);
	typedef void(__stdcall *OnEventTriggeredCallback)(tsm::IContext* context, tsm::IEvent* event);
	virtual void onEventTriggeredCallback(tsm::IContext* context, tsm::IEvent* event);
	// IStateMonitor::onEventHandling()
	delegate void OnEventHandlingDelegate(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current);
	typedef void(__stdcall *OnEventHandlingCallback)(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current);
	virtual void onEventHandlingCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current);
	// IStateMonitor::onStateChanged()
	delegate void OnStateChangedDelegate(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next);
	typedef void(__stdcall *OnStateChangedCallback)(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next);
	virtual void onStateChangedCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next);
#pragma endregion

	NativeType* get() { return m_nativeStateMonitor; }

protected:
	NativeType* m_nativeStateMonitor;
	IStateMonitor^ m_stateMonitor;
};

public ref class Context
{
	void construct(bool isAsync);

public:
	using NativeType = native::Context;

	Context() { construct(true); }
	Context(bool isAsync) { construct(isAsync); }
	virtual ~Context();
	!Context();

	bool isAsync();
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
	NativeType* get() { return m_nativeContext; }

protected:
	NativeType* m_nativeContext;
	tsm_NET::IStateMonitor^ m_stateMonitor;
	StateMonitorCaller^ m_stateMonitorCaller;
};

public ref class State
{
public:
	using NativeType = native::State;

	State() : State(nullptr) {}
	State(State^ masterState);
	virtual ~State();
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
#pragma endregion

internal:
	NativeType* get() { return m_nativeState; }

#pragma region Definition of delegate, callback signature and callback method. See native::Callback<> template class.
	delegate HRESULT HandleEventDelegate(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState);
	typedef HRESULT (__stdcall *HandleEventCallback)(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState);
	virtual HRESULT handleEventCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState);

	delegate HRESULT EntryDelegate(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState);
	typedef HRESULT (__stdcall *EntryCallback)(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState);
	virtual HRESULT entryCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState);

	delegate HRESULT ExitDelegate(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState);
	typedef HRESULT(__stdcall *ExitCallback)(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState);
	virtual HRESULT exitCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState);
#pragma endregion

protected:
	NativeType* m_nativeState;
};

public ref class Event
{
public:
	using NativeType = native::Event;

	Event();
	virtual ~Event();
	!Event();

#pragma region Methods to be implemented by sub class.
	virtual HResult preHandle(Context^ context) { return HResult::Ok; }
	virtual HResult postHandle(Context^ context, HResult hr) { return hr; }
#pragma endregion

	//
	// TODO: Implement setTimer() method.
	//

internal:
	NativeType* get() { return m_nativeEvent; }

#pragma region Definition of delegate, callback signature and callback method. See tsm::ICallback<> template class.
	delegate HRESULT PreHandleDelegate(tsm::IContext* context);
	typedef HRESULT(__stdcall *PreHandleCallback)(tsm::IContext* context);
	virtual HRESULT preHandleCallback(tsm::IContext* context);

	delegate HRESULT PostHandleDelegate(tsm::IContext* context, HRESULT hr);
	typedef HRESULT(__stdcall *PostHandleCallback)(tsm::IContext* context, HRESULT hr);
	virtual HRESULT postHandleCallback(tsm::IContext* context, HRESULT hr);
#pragma endregion

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
