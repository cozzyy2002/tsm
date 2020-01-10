#include "stdafx.h"
#include "GenericObject.h"
#include "NativeObjects.h"

namespace tsm_NET
{
using namespace common;

namespace Generic
{
	generic<typename E, typename S>
	StateMonitorCaller<E, S>::StateMonitorCaller(IStateMonitor<E, S>^ stateMonitor)
		: tsm_NET::StateMonitorCaller(nullptr)
		, m_stateMonitor(stateMonitor)
	{
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onIdleCallback(tsm::IContext* context)
	{
		m_stateMonitor->onIdle((Context<E, S>^)getManaged((native::Context*)context));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onEventTriggeredCallback(tsm::IContext* context, tsm::IEvent* event)
	{
		m_stateMonitor->onEventTriggered((Context<E, S>^)getManaged((native::Context*)context), (E)getManaged((native::Event*)event));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onEventHandlingCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
	{
		m_stateMonitor->onEventHandling((Context<E, S>^)getManaged((native::Context*)context), (E)getManaged((native::Event*)event), (S)getManaged((native::State*)current));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onStateChangedCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
	{
		m_stateMonitor->onStateChanged((Context<E, S>^)getManaged((native::Context*)context), (E)getManaged((native::Event*)event), (S)getManaged((native::State*)previous), (S)getManaged((native::State*)next));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onTimerStartedCallback(tsm::IContext* context, tsm::IEvent* event)
	{
		m_stateMonitor->onTimerStarted((Context<E, S>^)getManaged((native::Context*)context), (E)getManaged((native::Event*)event));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onWorkerThreadExitCallback(tsm::IContext* context, HRESULT exitCode)
	{
		m_stateMonitor->onWorkerThreadExit((Context<E, S>^)getManaged((native::Context*)context), (HResult)exitCode);
	}

	generic<typename E, typename S>
	void Context<E, S>::StateMonitor::set(IStateMonitor<E, S>^ value)
	{
		m_stateMonitor = value;
		if(value) {
			m_stateMonitorCaller = gcnew StateMonitorCaller<E, S>(value);
			m_nativeContext->setStateMonitor(m_stateMonitorCaller->get());
		} else {
			m_nativeContext->setStateMonitor(nullptr);
			delete m_stateMonitorCaller;
			m_stateMonitorCaller = nullptr;
		}
	}

	generic<typename C, typename E, typename S>
	tsm_NET::HResult State<C, E, S>::handleEvent(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^% nextState)
	{
		S _nextState;
		auto hr = handleEvent((C)context, (E)event, _nextState);
		if(_nextState) {
			nextState = (S)_nextState;
		}
		return (tsm_NET::HResult)hr;
	}

	generic<typename C, typename E, typename S>
		tsm_NET::HResult State<C, E, S>::entry(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^ previousState)
	{
		return (tsm_NET::HResult)entry((C)context, (E)event, (S)previousState);
	}

	generic<typename C, typename E, typename S>
	tsm_NET::HResult State<C, E, S>::exit(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^ nextState)
	{
		return (tsm_NET::HResult)exit((C)context, (E)event, (S)nextState);
	}

	generic<typename C>
	tsm_NET::HResult Event<C>::preHandle(tsm_NET::Context^ context)
	{
		return (tsm_NET::HResult)preHandle((C)context);
	}

	generic<typename C>
	tsm_NET::HResult Event<C>::postHandle(tsm_NET::Context^ context, tsm_NET::HResult hr)
	{
		return (tsm_NET::HResult)postHandle((C)context, (HResult)hr);
	}
}
}
