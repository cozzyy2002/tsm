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

void StateMonitorCaller::onTimerStoppedCallback(tsm::IContext* context, tsm::IEvent* event, HRESULT hr)
{
	m_stateMonitor->onTimerStopped(getManaged((native::Context*)context), getManaged((native::Event*)event), (HResult)hr);
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

HResult Context::shutdonw(int timeout_msec)
{
	return (HResult)m_nativeContext->shutdown(timeout_msec);
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

HResult Context::waitReady(int timeout_msec)
{
	return (HResult)m_nativeContext->waitReady(timeout_msec);
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

void Event::setDelayTimer(TimerClient^ client, TimeSpan delayTime)
{
	setTimer(client->getTimerClient(), (int)delayTime.TotalMilliseconds, 0);
}

void Event::setIntervalTimer(TimerClient^ client, TimeSpan intervalTime)
{
	setTimer(client->getTimerClient(), 0, (int)intervalTime.TotalMilliseconds);
}

void Event::setTimer(TimerClient^ client, TimeSpan delayTime, TimeSpan intervalTime)
{
	setTimer(client->getTimerClient(), (int)delayTime.TotalMilliseconds, (int)intervalTime.TotalMilliseconds);
}

void Event::setDelayTimer(TimerClient^ client, int delayTime_msec)
{
	setTimer(client->getTimerClient(), delayTime_msec, 0);
}

void Event::setIntervalTimer(TimerClient^ client, int intervalTime_msec)
{
	setTimer(client->getTimerClient(), 0, intervalTime_msec);
}

void Event::setTimer(TimerClient^ client, int delayTime_msec, int intervalTime_msec)
{
	setTimer(client->getTimerClient(), delayTime_msec, intervalTime_msec);
}

void Event::setTimer(tsm::TimerClient* timerClient, int delayTime, int intervalTime)
{
	m_nativeEvent->setTimer(timerClient, delayTime, intervalTime);
}

HResult Event::cancelTimer(int timeout)
{
	auto client = m_nativeEvent->_getTimerClient();
	return client ? (HResult)client->cancelEventTimer(m_nativeEvent, timeout) : HResult::IllegalMethodCall;
}

TimeSpan Event::DelayTime::get()
{
	return TimeSpan::FromMilliseconds(m_nativeEvent->_getDelayTime());
}

TimeSpan Event::InterValTime::get()
{
	return TimeSpan::FromMilliseconds(m_nativeEvent->_getIntervalTime());
}

int Event::TimeoutCount::get()
{
	return m_nativeEvent->_getTimeoutCount();
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

Error::Error(HRESULT hr)
	: m_hr(hr), m_message(nullptr)
{
}

Error::Error(tsm_NET::HResult hr)
	: m_hr((HRESULT)hr), m_message(nullptr)
{
}

HRESULT Error::HResult::get()
{
	return m_hr;
}

String^ Error::Message::get()
{
	if(m_message == nullptr) {
		auto hModule = tsm::GetStateMachineModule();
		auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		LPTSTR message;
		va_list args;
		FormatMessage(flags, (LPCVOID)hModule, m_hr, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPTSTR)&message, 100, &args);
		m_message = gcnew String(message);
		LocalFree(message);
	}
	return m_message;
}
