#include "stdafx.h"
#include "NativeObjects.h"

using namespace native;
using namespace tsm_NET::common;

// Dummy difinition to suppress `warning LNK4248: unresolved typeref token`
namespace tsm
{
struct ContextHandle {};
struct StateHandle {};
struct EventHandle {};
struct TimerHandle {};
}

Context::Context(ManagedType^ context, bool isAsync /*= true*/)
	: m_isAsync(isAsync)
	, m_managedContext(context)
{
}

State::State(ManagedType^ state, ManagedType^ masterState)
	: tsm::State<Context, Event, State>(getNative(masterState))
	, m_managedState(state)
	, m_handleEventCallback(state, gcnew ManagedType::HandleEventDelegate(state, &ManagedType::handleEventCallback))
	, m_entryCallback(state, gcnew ManagedType::EntryDelegate(state, &ManagedType::entryCallback))
	, m_exitCallback(state, gcnew ManagedType::ExitDelegate(state, &ManagedType::exitCallback))
{
}

HRESULT State::handleEvent(Context* context, Event* event, State** nextState)
{
	return m_handleEventCallback(context, event, nextState);
}

HRESULT State::entry(Context* context, Event* event, State* previousState)
{
	return m_entryCallback(context, event, previousState);
}

HRESULT State::exit(Context* context, Event* event, State* nextState)
{
	return m_exitCallback(context, event, nextState);
}

Event::Event(ManagedType^ event)
	: m_managedEvent(event)
	, m_preHandleCallback(event, gcnew ManagedType::PreHandleDelegate(event, &ManagedType::preHandleCallback))
	, m_postHandleCallback(event, gcnew ManagedType::PostHandleDelegate(event, &ManagedType::postHandleCallback))
{
}

HRESULT Event::preHandle(Context* context)
{
	return m_preHandleCallback(context);
}

HRESULT Event::postHandle(Context* context, HRESULT hr)
{
	return m_postHandleCallback(context, hr);
}
