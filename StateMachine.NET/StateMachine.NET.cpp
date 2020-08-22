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

//-------------- Managed TimerClient class. --------------------//
IList<Event^>^ TimerClient::PendingEvents::get()
{
	auto ret = gcnew List<Event^>();
	auto timerClient = getTimerClient();
	for(auto e : timerClient->getPendingEvents()) {
		ret->Add(getManaged((native::Event*)e.p));
	}
	return ret;
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

HResult AsyncContext::getAsyncExitCode(HResult% hrExitCode)
{
	HRESULT _hrExitCode;
	auto hr = tsm_NET::getAsyncExitCode(m_nativeContext, &_hrExitCode);
	if(SUCCEEDED(hr)) { hrExitCode = (HResult)_hrExitCode; }
	return (HResult)hr;
}

HRESULT tsm_NET::getAsyncExitCode(native::Context* context, HRESULT* phr)
{
	return context->getAsyncExitCode(phr);
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

tsm::TimerClient* Context::getTimerClient()
{
	return get()->_getTimerClient();
}

//-------------- Managed State class. --------------------//
void State::construct(State^ masterState, bool autoDispose)
{
	m_nativeState = new native::State(this, masterState, autoDispose);

	if(!m_nativeState->m_autoDispose) {
		// Prevent native object from deleting automatically.
		m_nativeState->AddRef();
	}

	IsExitCalledOnShutdown = false;
}

State^ State::getMasterState()
{
	return getManaged(m_nativeState->getMasterState());
}

State::~State()
{
	this->!State();
}

State::!State()
{
	if(m_nativeState && (!m_nativeState->m_autoDispose)) {
		// Delete native object.
		m_nativeState->Release();
	}
	m_nativeState = nullptr;
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

tsm::TimerClient* State::getTimerClient()
{
	return get()->_getTimerClient();
}

bool State::AutoDispose::get()
{
	return m_nativeState->m_autoDispose;
}

//-------------- Managed Event class. --------------------//
void Event::construct(int priority, bool autoDispose)
{
	m_nativeEvent = new native::Event(this, priority, autoDispose);

	if(!m_nativeEvent->m_autoDispose) {
		// Prevent native object from deleting automatically.
		m_nativeEvent->AddRef();
	}
}

Event::~Event()
{
	this->!Event();
}

Event::!Event()
{
	if(m_nativeEvent && (!m_nativeEvent->m_autoDispose)) {
		// Delete native object.
		m_nativeEvent->Release();
	}
	m_nativeEvent = nullptr;
}

void Event::setDelayTimer(Context^ context, TimeSpan delayTime)
{
	setTimer(context->get(), (int)delayTime.TotalMilliseconds, 0);
}

void Event::setIntervalTimer(Context^ context, TimeSpan intervalTime)
{
	setTimer(context->get(), 0, (int)intervalTime.TotalMilliseconds);
}

void Event::setTimer(Context^ context, TimeSpan delayTime, TimeSpan intervalTime)
{
	setTimer(context->get(), (int)delayTime.TotalMilliseconds, (int)intervalTime.TotalMilliseconds);
}

void Event::setDelayTimer(State^ state, TimeSpan delayTime)
{
	setTimer(state->get(), (int)delayTime.TotalMilliseconds, 0);
}

void Event::setIntervalTimer(State^ state, TimeSpan intervalTime)
{
	setTimer(state->get(), 0, (int)intervalTime.TotalMilliseconds);
}

void Event::setTimer(State^ state, TimeSpan delayTime, TimeSpan intervalTime)
{
	setTimer(state->get(), (int)delayTime.TotalMilliseconds, (int)intervalTime.TotalMilliseconds);
}

void Event::setTimer(tsm::TimerClient* timerClient, int delayTime, int intervalTime)
{
	m_nativeEvent->setTimer(timerClient, delayTime, intervalTime);
}

TimeSpan Event::DelayTime::get()
{
	return TimeSpan::FromMilliseconds(m_nativeEvent->_getDelayTime());
}

TimeSpan Event::InterValTime::get()
{
	return TimeSpan::FromMilliseconds(m_nativeEvent->_getIntervalTime());
}

int Event::Priority::get()
{
	return m_nativeEvent->_getPriority();
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

bool Event::AutoDispose::get()
{
	return m_nativeEvent->m_autoDispose;
}
