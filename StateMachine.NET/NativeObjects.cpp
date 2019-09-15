#include "stdafx.h"
#include "NativeObjects.h"

using namespace native;

// Dummy difinition to suppress `warning LNK4248: unresolved typeref token`
namespace tsm
{
struct ContextHandle {};
struct StateHandle {};
struct EventHandle {};
struct TimerHandle {};
}

template<class C>
typename C::ManagedType^ getManaged(C* native)
{
	return native ? native->get() : nullptr;
}

State::State(ManagedType^ state, ManagedType^ masterState)
	: tsm::State<Context, Event, State>(masterState ? masterState->get() : nullptr)
{
}

HRESULT State::handleEvent(Context* context, Event* event, State** nextState)
{
	tsm_NET::State^ _nextState = nullptr;
	auto ret = (HRESULT)m_managedState->handleEvent(getManaged(context), getManaged(event), *_nextState);
	if(_nextState != nullptr) {
		*nextState = _nextState->get();
	}

	return ret;
}

HRESULT State::entry(Context* context, Event* event, State* previousState)
{
	return (HRESULT)m_managedState->entry(getManaged(context), getManaged(event), getManaged(previousState));
}

HRESULT State::exit(Context* context, Event* event, State* nextState)
{
	return (HRESULT)m_managedState->exit(getManaged(context), getManaged(event), getManaged(nextState));
}

HRESULT Event::preHandle(Context* context)
{
	return (HRESULT)m_managedEvent->preHandle(getManaged(context));
}

HRESULT Event::postHandle(Context* context, HRESULT hr)
{
	return (HRESULT)m_managedEvent->postHandle(getManaged(context), (tsm_NET::HResult)hr);
}
