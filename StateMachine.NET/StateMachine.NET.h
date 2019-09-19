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

	Context(bool isAsync);
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
	virtual HResult handleEvent(Context^ context, Event^ event, State^% nextState) { return HResult::Ok; }
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

#pragma region Definition of delegate, callback signature and callback method. See native::Callback<> template class.
	delegate HRESULT HandleEventDelegate(native::Context* context, native::Event* event, native::State** nextState);
	typedef HRESULT (__stdcall *HandleEventCallback)(native::Context* context, native::Event* event, native::State** nextState);
	HRESULT handleEventCallback(native::Context* context, native::Event* event, native::State** nextState);

	delegate HRESULT EntryDelegate(native::Context* context, native::Event* event, native::State* previousState);
	typedef HRESULT (__stdcall *EntryCallback)(native::Context* context, native::Event* event, native::State* previousState);
	HRESULT entryCallback(native::Context* context, native::Event* event, native::State* previousState);

	delegate HRESULT ExitDelegate(native::Context* context, native::Event* event, native::State* nextState);
	typedef HRESULT(__stdcall *ExitCallback)(native::Context* context, native::Event* event, native::State* nextState);
	HRESULT exitCallback(native::Context* context, native::Event* event, native::State* nextState);
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

#pragma region Definition of delegate, callback signature and callback method. See native::Callback<> template class.
	delegate HRESULT PreHandleDelegate(native::Context* context);
	typedef HRESULT(__stdcall *PreHandleCallback)(native::Context* context);
	HRESULT preHandleCallback(native::Context* context);

	delegate HRESULT PostHandleDelegate(native::Context* context, HRESULT hr);
	typedef HRESULT(__stdcall *PostHandleCallback)(native::Context* context, HRESULT hr);
	HRESULT postHandleCallback(native::Context* context, HRESULT hr);
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
