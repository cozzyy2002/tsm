#include "stdafx.h"

#include "StateMachine.NET.h"
#include "NativeObjects.h"
#include "GenericObject.h"

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

::CultureInfo^ Error::CultureInfo::get()
{
	return s_cultureInfo;
}

void Error::CultureInfo::set(::CultureInfo^ value)
{
	s_cultureInfo = value;
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
		auto langId = (s_cultureInfo != nullptr) ? s_cultureInfo->LCID : 0;
		LPTSTR message;
		va_list args;
		FormatMessage(flags, (LPCVOID)hModule, m_hr, langId, (LPTSTR)&message, 100, &args);
		m_message = gcnew String(message);
		LocalFree(message);
	}
	return m_message;
}
