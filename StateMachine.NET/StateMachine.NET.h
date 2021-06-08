#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace Runtime::InteropServices;
using namespace System::Globalization;

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

interface class IContext;
interface class IState;
interface class IEvent;

public interface class IStateMonitor
{
	void onIdle(IContext^ context);
	void onEventTriggered(IContext^ context, IEvent^ event);
	void onEventHandling(IContext^ context, IEvent^ event, IState^ current);
	void onStateChanged(IContext^ context, IEvent^ event, IState^ previous, IState^ next);
	void onTimerStarted(IContext^ context, IEvent^ event) = 0;
	void onTimerStopped(IContext^ context, IEvent^ event, HResult hr) = 0;
	void onWorkerThreadExit(IContext^ context, HResult exitCode) = 0;

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

public interface class ITimerOwner
{
public:
	property IList<IEvent^>^ PendingEvents { IList<IEvent^>^ get(); }

	tsm::ITimerClient* getTimerClient();
};

public interface class IContext : public ITimerOwner
{
	using NativeType = native::Context;

	property bool UseNativeThread { bool get(); }
};

public interface class IState : public ITimerOwner
{
	using NativeType = native::State;

	HResult _handleEvent(IContext^ context, IEvent^ event, IState^% nextState);
	HResult _entry(IContext^ context, IEvent^ event, IState^ previousState);
	HResult _exit(IContext^ context, IEvent^ event, IState^ nextState);
	property bool IsExitCalledOnShutdown { bool get(); }
	NativeType* get();
};

public interface class IEvent
{
	using NativeType = native::Event;

	HResult _preHandle(IContext^ context);
	HResult _postHandle(IContext^ context, HResult hr);
	NativeType* get();
};

public interface class IAutoDisposable
{
	/**
	 * AutoDispose property determine the way to dispose this object.
	 * If this value is true(default)
	 *	* This object is disposed by State Machine when the object is no longer needed.
	 * else
	 *	* StateMachine never disposes Event object.
	 *	* The object should be disposed by user.
	 *
	 * This value is specified by autoDispose argument of constructor.
	 */
	property bool AutoDispose { bool get(); }
};

extern HRESULT getAsyncExitCode(native::Context* context, HRESULT* phr);

public ref class Error
{
public:
	static property CultureInfo^ CultureInfo { ::CultureInfo^ get(); void set(::CultureInfo^ value); }
	Error(HRESULT hr);
	Error(HResult hr);
	property HRESULT HResult { HRESULT get(); }
	property String^ Message { String^ get(); }

protected:
	static ::CultureInfo^ s_cultureInfo;
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
