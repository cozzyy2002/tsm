#include <StateMachine/stdafx.h>

#include "Handles.h"

namespace tsm {

#pragma region Implementation of _createHandle() method

template<>
EventHandle* HandleOwner<IEvent, EventHandle>::_createHandle(IEvent*)
{
	auto handle = new EventHandle();
	handle->isTimerCreated = false;

	return handle;
}

template<>
StateHandle* HandleOwner<IState, StateHandle>::_createHandle(IState*)
{
	auto handle = new StateHandle();

	return handle;
}

template<>
ContextHandle* HandleOwner<IContext, ContextHandle>::_createHandle(IContext* context)
{
	auto handle = new ContextHandle();
	if(context->isAsync()) {
		handle->_asyncData.reset(new ContextHandle::AsyncData());
	}
	handle->asyncData = handle->_asyncData.get();

	return handle;
}

template<>
TimerHandle* HandleOwner<TimerClient, TimerHandle>::_createHandle(TimerClient*)
{
	auto handle = new TimerHandle();
	handle->hTimerQueue = NULL;

	return handle;
}

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
