#include "stdafx.h"
#include "NativeObjects.h"

using namespace native;
using namespace tsm_NET::common;
using namespace System::Diagnostics;

// Dummy difinition to suppress `warning LNK4248: unresolved typeref token`
namespace tsm
{
struct ContextHandle {};
struct StateHandle {};
struct EventHandle {};
struct TimerHandle {};
}

StateMonitor::StateMonitor(StateMonitor::OwnerType^ owner,
				OwnerType::OnIdleCallback onIdleCallback,
				OwnerType::OnEventTriggeredCallback onEventTriggeredCallback,
				OwnerType::OnEventHandlingCallback onEventHandlingCallback,
				OwnerType::OnStateChangedCallback onStateChangedCallback,
				OwnerType::OnTimerStartedCallback onTimerStartedCallback,
				OwnerType::OnWorkerThreadExitCallback onWorkerThreadExitCallback)
	: m_onIdleCallback(onIdleCallback)
	, m_onEventTriggeredCallback(onEventTriggeredCallback)
	, m_onEventHandlingCallback(onEventHandlingCallback)
	, m_onStateChangedCallback(onStateChangedCallback)
	, m_onTimerStartedCallback(onTimerStartedCallback)
	, m_onWorkerThreadExitCallback(onWorkerThreadExitCallback)
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
	m_onWorkerThreadExitCallback(context, exitCode);
}

Context::Context(ManagedType^ context, bool isAsync /*= true*/)
	: m_isAsync(isAsync)
	, m_managedContext(context)
	, m_stateMonitor(nullptr)
{
}

State::State(ManagedType^ state, ManagedType^ masterState,
				ManagedType::HandleEventCallback handleEventCallback,
				ManagedType::EntryCallback entryCallback,
				ManagedType::ExitCallback exitCallback)
	: m_managedState(state)
	, m_masterState(getNative(masterState))
	, m_handleEventCallback(handleEventCallback)
	, m_entryCallback(entryCallback)
	, m_exitCallback(exitCallback)
{
}

State::~State()
{
	delete m_managedState;
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
	Trace::WriteLine(String::Format("native::State::_exit(): Current AppDomain={0}", System::AppDomain::CurrentDomain->FriendlyName));
	return m_exitCallback(context, event, nextState);
}

bool State::_isExitCalledOnShutdown() const
{
	return m_managedState->IsExitCalledOnShutdown;
}

Event::Event(ManagedType^ event,
				ManagedType::PreHandleCallback preHandleCallback,
				ManagedType::PostHandleCallback postHandleCallback)
	: m_managedEvent(event)
	, m_preHandleCallback(preHandleCallback)
	, m_postHandleCallback(postHandleCallback)
	, m_timerClient(nullptr)
{
}

Event::~Event()
{
	delete m_managedEvent;
}

HRESULT Event::_preHandle(tsm::IContext* context)
{
	return m_preHandleCallback(context);
}

HRESULT Event::_postHandle(tsm::IContext* context, HRESULT hr)
{
	return m_postHandleCallback(context, hr);
}
