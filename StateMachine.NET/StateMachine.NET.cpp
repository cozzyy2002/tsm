#include "stdafx.h"

#include "StateMachine.NET.h"
#include "NativeObjects.h"

using namespace tsm_NET;
using namespace tsm_NET::common;
using namespace System::Diagnostics;

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
void Context::construct(bool isAsync, bool useNativeThread)
{
	m_useNativeThread = useNativeThread;
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
		m_nativeContext->setStateMonitor(nullptr);
		delete m_stateMonitorCaller;
		m_stateMonitorCaller = nullptr;
	}
}

//-------------- Managed State class. --------------------//
State::State(State^ masterState)
{
	m_nativeState = new native::State(this, masterState);

	IsExitCalledOnShutdown = false;
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

long State::SequenceNumber::get()
{
	return m_nativeState->getSequenceNumber();
}

int State::MemoryWeight::get()
{
	return tsm::IState::getMemoryWeight();
}

void State::MemoryWeight::set(int value)
{
	tsm::IState::setMemoryWeight(value);
}

//-------------- Managed Event class. --------------------//
Event::Event()
{
	m_nativeEvent = new native::Event(this);
	//m_nativeEvent->AddRef();
}

long Event::SequenceNumber::get()
{
	return m_nativeEvent->getSequenceNumber();
}

int Event::MemoryWeight::get()
{
	return tsm::IEvent::getMemoryWeight();
}

void Event::MemoryWeight::set(int value)
{
	tsm::IEvent::setMemoryWeight(value);
}
