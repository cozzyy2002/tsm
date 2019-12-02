#include "stdafx.h"

#include "StateMachine.NET.h"
#include "NativeObjects.h"

using namespace tsm_NET;
using namespace tsm_NET::common;

///*static*/ event EventHandler<IStateMonitor::AssertFailedEventArgs<HResult>^>^ IStateMonitor::AssertFailedEvent;

//-------------- Implementation of IStateMonitorCaller. --------------------//
StateMonitorCaller::StateMonitorCaller(IStateMonitor^ stateMonitor)
	: m_stateMonitor(stateMonitor)
{
	m_nativeStateMonitor = new native::StateMonitor(this);
}

StateMonitorCaller::~StateMonitorCaller()
{
	this->!StateMonitorCaller();
}

StateMonitorCaller::!StateMonitorCaller()
{
	if(m_nativeStateMonitor) {
		delete m_nativeStateMonitor;
		m_nativeStateMonitor = nullptr;
	}
}

void StateMonitorCaller::onIdleCallback(tsm::IContext* context)
{
	m_stateMonitor->onIdle(getManaged((native::Context*)context));
}

void StateMonitorCaller::onEventTriggeredCallback(tsm::IContext* context, tsm::IEvent* event)
{
	m_stateMonitor->onEventTriggered(getManaged((native::Context*)context), getManaged((native::Event*)event));
}

void StateMonitorCaller::onEventHandlingCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
{
	m_stateMonitor->onEventHandling(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)current));
}

void StateMonitorCaller::onStateChangedCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
{
	m_stateMonitor->onStateChanged(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)previous), getManaged((native::State*)next));
}

void StateMonitorCaller::onTimerStartedCallback(tsm::IContext* context, tsm::IEvent* event)
{
	m_stateMonitor->onTimerStarted(getManaged((native::Context*)context), getManaged((native::Event*)event));
}

void StateMonitorCaller::onWorkerThreadExitCallback(tsm::IContext* context, HRESULT exitCode)
{
	m_stateMonitor->onWorkerThreadExit(getManaged((native::Context*)context), (HResult)exitCode);
}

//-------------- Managed Context class. --------------------//
void Context::construct(bool isAsync)
{
	m_nativeContext = new native::Context(this, isAsync);
}

Context::~Context()
{
	this->!Context();
}

Context::!Context()
{
	if(m_nativeContext) {
		delete m_nativeContext;
		m_nativeContext = nullptr;
	}
}

bool Context::isAsync()
{
	return m_nativeContext->isAsync();
}

HResult Context::setup(State^ initialState, Event^ event)
{
	return (HResult)m_nativeContext->setup(getNative(initialState), getNative(event));
}

HResult Context::shutdown(TimeSpan timeout)
{
	return (HResult)m_nativeContext->shutdown((DWORD)timeout.TotalMilliseconds);
}

HResult Context::triggerEvent(Event^ event)
{
	return (HResult)m_nativeContext->triggerEvent(getNative(event));
}

HResult Context::handleEvent(Event^ event)
{
	return (HResult)m_nativeContext->handleEvent(getNative(event));
}

HResult Context::waitReady(TimeSpan timeout)
{
	return (HResult)m_nativeContext->waitReady((DWORD)timeout.TotalMilliseconds);
}

State^ Context::getCurrentState()
{
	return getManaged(m_nativeContext->getCurrentState());
}

void Context::StateMonitor::set(IStateMonitor^ value)
{
	m_stateMonitor = value;
	if(value) {
		m_stateMonitorCaller = gcnew StateMonitorCaller(value);
		m_nativeContext->setStateMonitor(m_stateMonitorCaller->get());
	} else {
		delete m_stateMonitorCaller;
		m_stateMonitorCaller = nullptr;
		m_nativeContext->setStateMonitor(nullptr);
	}
}

//-------------- Managed State class. --------------------//
State::State(State^ masterState)
{
	m_nativeState = new native::State(this, masterState);
	m_nativeState->AddRef();
	IsExitCalledOnShutdown = false;
}

State::~State()
{
	this->!State();
}

State::!State()
{
	if(m_nativeState)
	{
		m_nativeState->Release();
		m_nativeState = nullptr;
	}
}

HRESULT State::handleEventCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState)
{
	State^ _nextState = nullptr;
	auto hr = handleEvent(getManaged((native::Context*)context), getManaged((native::Event*)event), _nextState);
	if(_nextState) {
		*nextState = getNative(_nextState);
	}
	return (HRESULT)hr;
}

HRESULT State::entryCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState)
{
	return (HRESULT)entry(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)previousState));
}

HRESULT State::exitCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState)
{
	return (HRESULT)exit(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)nextState));
}

State^ State::getMasterState()
{
	return getManaged(m_nativeState->getMasterState());
}

//State^ State::getSubState()
//{
//	auto subState = m_nativeState->getSubState();
//	return subState ? subState->get() : nullptr;
//}

bool State::isSubState()
{
	return m_nativeState->isSubState();
}

//-------------- Managed Event class. --------------------//
Event::Event()
{
	m_nativeEvent = new native::Event(this);
	m_nativeEvent->AddRef();
}

Event::~Event()
{
	this->!Event();
}

Event::!Event()
{
	if(m_nativeEvent)
	{
		m_nativeEvent->Release();
		m_nativeEvent = nullptr;
	}
}

HRESULT Event::preHandleCallback(tsm::IContext* context)
{
	return (HRESULT)preHandle(getManaged((native::Context*)context));
}

HRESULT Event::postHandleCallback(tsm::IContext* context, HRESULT hr)
{
	return (HRESULT)postHandle(getManaged((native::Context*)context), (HResult)hr);
}
