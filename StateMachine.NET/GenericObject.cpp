#include "stdafx.h"
#include "GenericObject.h"
#include "NativeObjects.h"

namespace tsm_NET
{
using namespace common;

namespace Generic
{
	// Internal IStateMonitor implemantation that calls method of user's IStateMonitor.
	generic<typename C, typename E, typename S>
		where C : tsm_NET::Context
		where E : tsm_NET::Event
		where S : tsm_NET::State
	ref class StateMonitor : tsm_NET::IStateMonitor
	{
	internal:
		StateMonitor(IStateMonitor<C, E, S>^ stateMonitor) : m_stateMonitor(stateMonitor) {}

	public:
		virtual void onIdle(tsm_NET::Context^ context) {}
		virtual void onEventTriggered(tsm_NET::Context^ context, tsm_NET::Event^ event) {}
		virtual void onEventHandling(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^ current) {}
		virtual void onStateChanged(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^ previous, tsm_NET::State^ next) {
			m_stateMonitor->onStateChanged((C)context, (E)event, (S)previous, (S)next);
		}

	protected:
		IStateMonitor<C, E, S>^ m_stateMonitor;
	};

	generic<typename E, typename S>
	void Context<E, S>::StateMonitor::set(IStateMonitor<Context<E, S>^, E, S>^ value)
	{
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
