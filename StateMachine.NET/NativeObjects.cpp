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

StateMonitor::StateMonitor(StateMonitor::OwnerType^ owner)
	: m_onIdleCallback(gcnew OwnerType::OnIdleDelegate(owner, &OwnerType::onIdleCallback))
	, m_onEventTriggeredCallback(gcnew OwnerType::OnEventTriggeredDelegate(owner, &OwnerType::onEventTriggeredCallback))
	, m_onEventHandlingCallback(gcnew OwnerType::OnEventHandlingDelegate(owner, &OwnerType::onEventHandlingCallback))
	, m_onStateChangedCallback(gcnew OwnerType::OnStateChangedDelegate(owner, &OwnerType::onStateChangedCallback))
	, m_onTimerStartedCallback(gcnew OwnerType::OnTimerStartedDelegate(owner, &OwnerType::onTimerStartedCallback))
	, m_onWorkerThreadCallback(gcnew OwnerType::OnWorkerThreadExitDelegate(owner, &OwnerType::onWorkerThreadExitCallback))
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

void StateMonitor::onTimerStarted(tsm::IContext* context, tsm::IEvent* event)
{
	m_onTimerStartedCallback(context, event);
}

void StateMonitor::onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode)
{
	m_onWorkerThreadCallback(context, exitCode);
}

Context::Context(ManagedType^ context, bool isAsync /*= true*/)
	: m_isAsync(isAsync)
	, m_managedContext(context)
	, m_stateMonitor(nullptr)
{
}

State::State(ManagedType^ state, ManagedType^ masterState)
	: m_managedState(state)
	, m_masterState(getNative(masterState))
	, m_handleEventCallback(gcnew ManagedType::HandleEventDelegate(state, &ManagedType::handleEventCallback))
	, m_entryCallback(gcnew ManagedType::EntryDelegate(state, &ManagedType::entryCallback))
	, m_exitCallback(gcnew ManagedType::ExitDelegate(state, &ManagedType::exitCallback))
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

bool State::_callExitOnShutdown() const
{
	return m_managedState->CallExitOnShutdown;
}

Event::Event(ManagedType^ event)
	: m_managedEvent(event)
	, m_preHandleCallback(gcnew ManagedType::PreHandleDelegate(event, &ManagedType::preHandleCallback))
	, m_postHandleCallback(gcnew ManagedType::PostHandleDelegate(event, &ManagedType::postHandleCallback))
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
