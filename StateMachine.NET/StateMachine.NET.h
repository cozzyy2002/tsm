#pragma once

using namespace System;

namespace native
{
class Context;
class State;
class Event;
}

namespace tsm_NET
{

ref class Context;
ref class State;
ref class Event;

public enum class HResult : int
{
	Ok = S_OK,
	False = S_FALSE,
};

public ref class Context
{
public:
	using NativeType = native::Context;

	Context();
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
	property bool IsAsync { bool get() { return isAsync(); }}
	property State^ CurrentState { State^ get() { return getCurrentState(); }}
#pragma endregion

internal:
	NativeType* get() { return m_nativeContext; }

protected:
	NativeType* m_nativeContext;
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
	virtual HResult handleEvent(Context^ context, Event^ event, State% nextState) { return HResult::Ok; }
	virtual HResult entry(Context^ context, Event^ event, State^ previousState) { return HResult::Ok; }
	virtual HResult exit(Context^ context, Event^ event, State^ nextState) { return HResult::Ok; }
#pragma endregion

	State^ getMasterState();
	//State^ getSubState();
	bool isSubState();

#pragma region .NET properties
	property State^ MasterState { State^ get() { return getMasterState(); }}
	property bool IsSubState { bool get() { return isSubState(); }}
#pragma endregion

internal:
	NativeType* get() { return m_nativeState; }

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

protected:
	NativeType* m_nativeEvent;
};
}
