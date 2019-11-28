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
	void Context<E, S>::StateMonitor::set(IStateMonitor<E, S>^ value)
	{
		m_stateMonitor = value;
		if(value) {
			m_stateMonitorCaller = gcnew StateMonitorCaller<E, S>(value);
			m_nativeContext->setStateMonitor(m_stateMonitorCaller->get());
		} else {
			delete m_stateMonitorCaller;
			m_stateMonitorCaller = nullptr;
			m_nativeContext->setStateMonitor(nullptr);
		}
	}

	generic<typename C, typename E, typename S>
	HRESULT State<C, E, S>::handleEventCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState)
	{
		S _nextState;
		auto hr = handleEvent((C)getManaged((native::Context*)context), (E)getManaged((native::Event*)event), _nextState);
		if(_nextState) {
			*nextState = _nextState->get();
		}
		return (HRESULT)hr;
	}

	generic<typename C, typename E, typename S>
	HRESULT State<C, E, S>::entryCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState)
	{
		return (HRESULT)entry((C)getManaged((native::Context*)context), (E)getManaged((native::Event*)event), (S)getManaged((native::State*)previousState));
	}

	generic<typename C, typename E, typename S>
	HRESULT State<C, E, S>::exitCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState)
	{
		return (HRESULT)exit((C)getManaged((native::Context*)context), (E)getManaged((native::Event*)event), (S)getManaged((native::State*)nextState));
	}

	generic<typename C>
	HRESULT Event<C>::preHandleCallback(tsm::IContext* context)
	{
		return (HRESULT)preHandle((C)getManaged((native::Context*)context));
	}

	generic<typename C>
	HRESULT Event<C>::postHandleCallback(tsm::IContext* context, HRESULT hr)
	{
		return (HRESULT)postHandle((C)getManaged((native::Context*)context), (HResult)hr);
	}
}
}
