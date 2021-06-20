#pragma once

#include <StateMachine/Assert.h>

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
	using NativeType = tsm::IStateMonitor;

	void onIdle(IContext^ context);
	void onEventTriggered(IContext^ context, IEvent^ event);
	void onEventHandling(IContext^ context, IEvent^ event, IState^ current);
	void onStateChanged(IContext^ context, IEvent^ event, IState^ previous, IState^ next);
	void onTimerStarted(IContext^ context, IEvent^ event);
	void onTimerStopped(IContext^ context, IEvent^ event, HResult hr);
	void onWorkerThreadExit(IContext^ context, HResult exitCode);

	NativeType* get();

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

public ref class Assert
{
public:
	static Assert();

	delegate void OnAssertFailedProcDelegate(HResult hr, String^ exp, String^ sourceFile, int line);
	static property OnAssertFailedProcDelegate^ OnAssertFailedProc {
		OnAssertFailedProcDelegate^ get() { return onAssertFailedProc; }
		void set(OnAssertFailedProcDelegate^ value) { onAssertFailedProc = value; }
	}

	delegate void OnAssertFailedWriterDelegate(String^ msg);
	static property OnAssertFailedWriterDelegate^ OnAssertFailedWriter {
		OnAssertFailedWriterDelegate^ get() { return onAssertFailedWriter; }
		void set(OnAssertFailedWriterDelegate^ value) { onAssertFailedWriter = value; }
	}

protected:
	static OnAssertFailedProcDelegate^ onAssertFailedProc;
	static OnAssertFailedWriterDelegate^ onAssertFailedWriter;
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
