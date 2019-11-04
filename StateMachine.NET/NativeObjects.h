#pragma once

#include "StateMachine.NET.h"

using System::Runtime::InteropServices::Marshal;

namespace native
{
class Context;
class State;
class Event;

/*
	Callback template class.
	This class is used to avoid `Cannot pass a GCHandle across AppDomains` exception
	when method of managed class is called by native(unmanaged) class in worker thread.

	How to use this class.

	1. Define delegate(class D), callback signature(class C) and callback method in managed class(class M).
		delegate HRESULT XxxDelegate(...);				// delegate
		typedef HRESULT (__stdcall *XxxCallback)(...);	// callback signature
		HRESULT xxxCallback(...) { method body }		// callback method

	2. Declare member variable of this class in native class.
		Callback<ManagedClass, XxxDelegate, XxxCallback> m_xxxCallback;

	3. Initialize the member variable in the constructor of native class.
		m_xxxCallback(managedObject, gcnew XxxDelegate(managedObject, &ManagedClass::xxxCallback))

	4. Callback from native class.
		m_xxxCallback(...);

	NOTE:
		Parameters and return value should be native(unmanaged) type.

	See http://lambert.geek.nz/2007/05/unmanaged-appdomain-callback/.
*/
template<class M, class D, class C>
class Callback
{
public:
	Callback(M^ managed, D^ del) : del(del) {
		callback = (C)Marshal::GetFunctionPointerForDelegate(del).ToPointer();
	}

	operator C() { return callback; }

protected:
	gcroot<D^> del;
	C callback;
};

class Context : public tsm::AsyncContext<Event, State>
{
public:
	using ManagedType = tsm_NET::Context;

	Context(ManagedType^ context, bool isAsync = true);

	ManagedType^ get() { return m_managedContext; }

protected:
	gcroot<ManagedType^> m_managedContext;
};

class State : public tsm::State<Context, Event, State>
{
public:
	using ManagedType = tsm_NET::State;

	State(ManagedType^ state, ManagedType^ masterState);

#pragma region Methods that call method of Managed State class.
	virtual HRESULT handleEvent(Context* context, Event* event, State** nextState) override;
	virtual HRESULT entry(Context* context, Event* event, State* previousState) override;
	virtual HRESULT exit(Context* context, Event* event, State* nextState) override;
#pragma endregion

	ManagedType^ get() { return m_managedState; }

protected:
	gcroot<ManagedType^> m_managedState;

	Callback<ManagedType, ManagedType::HandleEventDelegate, ManagedType::HandleEventCallback> m_handleEventCallback;
	Callback<ManagedType, ManagedType::EntryDelegate, ManagedType::EntryCallback> m_entryCallback;
	Callback<ManagedType, ManagedType::ExitDelegate, ManagedType::ExitCallback> m_exitCallback;
};

class Event : public tsm::Event<Context>
{
public:
	using ManagedType = tsm_NET::Event;

	Event(ManagedType^ event);

#pragma region Methods that call method of Managed Event class.
	virtual HRESULT preHandle(Context* context) override;
	virtual HRESULT postHandle(Context* context, HRESULT hr) override;
#pragma endregion

	ManagedType^ get() { return m_managedEvent; }

protected:
	gcroot<ManagedType^> m_managedEvent;

	Callback<ManagedType, ManagedType::PreHandleDelegate, ManagedType::PreHandleCallback> m_preHandleCallback;
	Callback<ManagedType, ManagedType::PostHandleDelegate, ManagedType::PostHandleCallback> m_postHandleCallback;
};

}