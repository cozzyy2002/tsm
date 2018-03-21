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

		_asyncData->hWnd = NULL;
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

/**
 * Add the event to event queue and signal that event is available.
 * Events are added to the queue by priority order.
 * deque::back() returns event with highest priority.
 * Events with same priority are added by FIFO order(deque::back() returns event triggered first).
 */
HRESULT ContextHandle::AsyncData::queueEvent(IEvent* event)
{
	lock_t _lock(eventQueueLock);
	auto it = eventQueue.begin();
	for(; it != eventQueue.end(); it++) {
		if(event->_getPriority() <= (*it)->_getPriority()) break;
	}
	eventQueue.insert(it, event);

	return S_OK;
}

void ContextHandle::callStateMonitor(IContext* context, std::function<void(IContext* context, IStateMonitor* stateMonitor)> caller)
{
	auto stateMonitor = context->_getStateMonitor();
	if(stateMonitor) {
		caller(context, stateMonitor);
	}
}

}
