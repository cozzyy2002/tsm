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

StateMonitor::StateMonitor(StateMonitor::ManagedType^ managed, StateMonitor::OwnerType^ owner)
	: m_onIdleCallback(managed, gcnew OwnerType::OnIdleDelegate(owner, &OwnerType::onIdleCallback))
	, m_onEventTriggeredCallback(managed, gcnew OwnerType::OnEventTriggeredDelegate(owner, &OwnerType::onEventTriggeredCallback))
	, m_onEventHandlingCallback(managed, gcnew OwnerType::OnEventHandlingDelegate(owner, &OwnerType::onEventHandlingCallback))
	, m_onStateChangedCallback(managed, gcnew OwnerType::OnStateChangedDelegate(owner, &OwnerType::onStateChangedCallback))
{
}

void StateMonitor::onIdle(tsm::IContext* context)
{
	m_onIdleCallback(context);
}

void StateMonitor::onEventTriggered(tsm::IContext* context, tsm::IEvent* event)
{
	m_onEventTriggeredCallback(context, event);
}

void StateMonitor::onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
{
	m_onEventHandlingCallback(context, event, current);
}

void StateMonitor::onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
{
	m_onStateChangedCallback(context, event, previous, next);
}

Context::Context(ManagedType^ context, bool isAsync /*= true*/)
	: m_isAsync(isAsync)
	, m_managedContext(context)
	, m_stateMonitor(nullptr)
{
}

State::State(ManagedType^ state, ManagedType^ masterState)
	: m_managedState(state)
	, m_handleEventCallback(state, gcnew ManagedType::HandleEventDelegate(state, &ManagedType::handleEventCallback))
	, m_entryCallback(state, gcnew ManagedType::EntryDelegate(state, &ManagedType::entryCallback))
	, m_exitCallback(state, gcnew ManagedType::ExitDelegate(state, &ManagedType::exitCallback))
	, m_masterState(getNative(masterState))
{
}

HRESULT State::_handleEvent(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState)
{
	return m_handleEventCallback(context, event, nextState);
}

HRESULT State::_entry(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState)
{
	return m_entryCallback(context, event, previousState);
}

HRESULT State::_exit(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState)
{
	return m_exitCallback(context, event, nextState);
}

Event::Event(ManagedType^ event)
	: m_managedEvent(event)
	, m_preHandleCallback(event, gcnew ManagedType::PreHandleDelegate(event, &ManagedType::preHandleCallback))
	, m_postHandleCallback(event, gcnew ManagedType::PostHandleDelegate(event, &ManagedType::postHandleCallback))
	, m_timerClient(nullptr)
{
}

HRESULT Event::_preHandle(tsm::IContext* context)
{
	return m_preHandleCallback(context);
}

HRESULT Event::_postHandle(tsm::IContext* context, HRESULT hr)
{
	return m_postHandleCallback(context, hr);
}
