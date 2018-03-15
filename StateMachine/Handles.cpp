#include <StateMachine/stdafx.h>

#include "Handles.h"

namespace tsm {

EventHandle::EventHandle(IEvent*)
	: isTimerCreated(false)
{
}

StateHandle::StateHandle(IState*)
{
}

ContextHandle::ContextHandle(IContext* context)
{
	if(context->isAsync()) {
		_asyncData.reset(new AsyncData());
	}
	asyncData = _asyncData.get();
}

TimerHandle::TimerHandle(TimerClient*)
	: hTimerQueue(NULL)
{
}

#pragma region Implementation of _createHandle() method

#define IMPLEMTENT_CREATE_HANDLE(T, H) \
	template<> H* HandleOwner<T, H>::_createHandle(T* instance) { return new H(instance); }

IMPLEMTENT_CREATE_HANDLE(IEvent, EventHandle)
IMPLEMTENT_CREATE_HANDLE(IState, StateHandle)
IMPLEMTENT_CREATE_HANDLE(IContext, ContextHandle)
IMPLEMTENT_CREATE_HANDLE(TimerClient, TimerHandle)

#pragma endregion

#pragma region Implementation of _deleteHandle() method

#define IMPLEMENT_DELETE_HANDLE(T, H) \
	template<> void HandleOwner<T, H>::_deleteHandle(H* handle) { delete handle; }

IMPLEMENT_DELETE_HANDLE(IEvent, EventHandle)
IMPLEMENT_DELETE_HANDLE(IState, StateHandle)
IMPLEMENT_DELETE_HANDLE(IContext, ContextHandle)
IMPLEMENT_DELETE_HANDLE(TimerClient, TimerHandle)

#pragma endregion

}
