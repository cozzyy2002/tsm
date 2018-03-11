#include <StateMachine/stdafx.h>

#include "Handles.h"

namespace tsm {

template<>
EventHandle* HandleOwner<IEvent, EventHandle>::_createHandle(IEvent*)
{
	auto handle = new EventHandle();
	handle->isTimerCreated = false;

	return handle;
}

template<>
void HandleOwner<IEvent, EventHandle>::_deleteHandle(EventHandle* handle)
{
	delete handle;
}

template<>
StateHandle* HandleOwner<IState, StateHandle>::_createHandle(IState*)
{
	auto handle = new StateHandle();

	return handle;
}

template<>
void HandleOwner<IState, StateHandle>::_deleteHandle(StateHandle* handle)
{
	delete handle;
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
void HandleOwner<IContext, ContextHandle>::_deleteHandle(ContextHandle* handle)
{
	delete handle;
}

template<>
TimerHandle* HandleOwner<TimerClient, TimerHandle>::_createHandle(TimerClient*)
{
	auto handle = new TimerHandle();
	handle->hTimerQueue = NULL;

	return handle;
}

template<>
void HandleOwner<TimerClient, TimerHandle>::_deleteHandle(TimerHandle* handle)
{
	delete handle;
}

}
