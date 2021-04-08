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
{
}

#pragma region Implementation of HandleFactory class.

#define IMPLEMTENT_HANDLE_FACTORY(T, H) \
	template<> /*static*/ H* HandleOwner<T, H>::HandleFactory::create(T* instance) { return new H(instance); } \
	template<> void HandleOwner<T, H>::HandleFactory::operator()(H* handle) const { delete handle; }

IMPLEMTENT_HANDLE_FACTORY(IEvent, EventHandle)
IMPLEMTENT_HANDLE_FACTORY(IState, StateHandle)
IMPLEMTENT_HANDLE_FACTORY(IContext, ContextHandle)
IMPLEMTENT_HANDLE_FACTORY(TimerClient, TimerHandle)

#pragma endregion

ContextHandle::AsyncData::AsyncData()
{
	hWorkerThread = NULL;
	hWnd = NULL;
	msg = 0;
	appWndProc = nullptr;
}

/**
 * Add the event to event queue.
 * Events are added to the queue by ascending priority order.
 * That is First event has the lowest priority and was queued most recent.
 * Then deque::back() returns event with highest priority.
 * Events with same priority are added by FIFO order(deque::back() returns event triggered first).
 */
HRESULT ContextHandle::AsyncData::queueEvent(IEvent* event)
{
	lock_t _lock(eventQueueLock);
	auto it = eventQueue.begin();
	for(; it != eventQueue.end(); it++) {
		if(event->_comparePriority(*it) <= 0) { break; }
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
