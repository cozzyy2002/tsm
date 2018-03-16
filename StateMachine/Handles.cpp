#include <StateMachine/stdafx.h>

#include "Handles.h"

namespace tsm {

EventHandle::EventHandle(IEvent*)
	: isTimerCreated(false)
{
}

StateHandle::StateHandle(IState*)
	: entryCalled(false)
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

#pragma region Implementation of HandleFactory class.

#define IMPLEMTENT_HANDLE_FACTORY(T, H) \
	template<> H* HandleOwner<T, H>::HandleFactory::create(T* instance) { return new H(instance); } \
	template<> void HandleOwner<T, H>::HandleFactory::operator()(H* handle) const { delete handle; }

IMPLEMTENT_HANDLE_FACTORY(IEvent, EventHandle)
IMPLEMTENT_HANDLE_FACTORY(IState, StateHandle)
IMPLEMTENT_HANDLE_FACTORY(IContext, ContextHandle)
IMPLEMTENT_HANDLE_FACTORY(TimerClient, TimerHandle)

#pragma endregion

}
