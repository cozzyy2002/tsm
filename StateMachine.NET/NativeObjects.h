#pragma once

#include "StateMachine.NET.h"

namespace native
{
class Context;
class State;
class Event;

class Context : public tsm::AsyncContext<Event, State>
{
public:
	using ManagedType = tsm_NET::Context;

	Context(ManagedType^ context, bool isAsync = true)
		: m_managedContext(context) {}

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
};

class Event : public tsm::Event<Context>
{
public:
	using ManagedType = tsm_NET::Event;

	Event(ManagedType^ event) : m_managedEvent(event) {}

#pragma region Methods that call method of Managed Event class.
	virtual HRESULT preHandle(Context* context) override;
	virtual HRESULT postHandle(Context* context, HRESULT hr) override;
#pragma endregion

	ManagedType^ get() { return m_managedEvent; }

protected:
	gcroot<ManagedType^> m_managedEvent;
};

}
