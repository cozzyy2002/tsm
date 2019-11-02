#include "stdafx.h"
#include "GenericObject.h"
#include "NativeObjects.h"

namespace tsm_NET {
using namespace common;

namespace Generic {

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
