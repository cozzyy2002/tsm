#include <StateMachine/stdafx.h>

#include <StateMachine/TimerClient.h>
#include <StateMachine/Assert.h>

#include "Handles.h"

namespace tsm {

static VOID CALLBACK timerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired);

HRESULT TimerClient::cancelEventTimer(IEvent* event)
{
	auto th = _getHandle();

	// Check if any timer has started by _triggerDelayedEvent() method.
	HR_ASSERT(th->hTimerQueue, E_ILLEGAL_METHOD_CALL);

	// Preserve Timer object until callback will complete or will be canceled.
	CComPtr<TimerHandle::Timer> timer;
	auto hr = S_OK;
	{
		lock_t _lock(th->lock);

		auto it = th->timers.find(event);
		if(it != th->timers.end()) {
			timer = it->second;
			// Remove timer.
			th->timers.erase(it);
		} else {
			// Timer specified might have expired already.
			hr = S_FALSE;
		}
	}
	if(timer.p) {
		// Delete timer and wait for currently running timer callback function.
		WIN32_ASSERT(DeleteTimerQueueTimer(th->hTimerQueue, timer->hTimer, INVALID_HANDLE_VALUE));
	}
	return hr;
}

HRESULT TimerClient::cancelAllEventTimers()
{
	auto th = _getHandle();

	// In case TimerHandle::hTimerQueue is cleared in another thread.
	auto hTimerQueue = th->hTimerQueue;
	if(hTimerQueue) {
		{
			lock_t _lock(th->lock);
			th->hTimerQueue = NULL;
			th->timers.clear();
		}
		// Cancel and delete all pending timers and delete timer queue.
		WIN32_EXPECT(DeleteTimerQueueEx(hTimerQueue, INVALID_HANDLE_VALUE));
	}

	return S_OK;
}

HRESULT TimerClient::_setEventTimer(TimerType timerType, IContext* context, IEvent* event)
{
	auto th = _getHandle();

	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	// Ensure that timer is required.
	HR_ASSERT(event->_getTimerClient(), E_ILLEGAL_METHOD_CALL);

	// State machine should not call this method with event whose timer is created already.
	auto eh = event->_getHandle();
	HR_ASSERT(!eh->isTimerCreated, E_ILLEGAL_METHOD_CALL);
	eh->isTimerCreated = true;

	lock_t _lock(th->lock);

	if(!th->hTimerQueue) {
		// Create timer queue for first time.
		WIN32_ASSERT(th->hTimerQueue = CreateTimerQueue());
	}

	// Create timer object.
	CComPtr<TimerHandle::Timer> timer(new TimerHandle::Timer());
	timer->timerType = timerType;
	timer->context = context;
	timer->event = event;

	// Create timer-queue timer and pass timer object as parameter for timer callback function.
	ULONG flags = 0;
	flags |= ((timer->timerType == TimerType::HandleEvent) ? WT_EXECUTELONGFUNCTION : 0);
	flags |= ((event->_getIntervalTime() == 0) ? WT_EXECUTEONLYONCE : 0);
	WIN32_ASSERT(CreateTimerQueueTimer(&timer->hTimer, th->hTimerQueue, timerCallback, timer.p, event->_getDelayTime(), event->_getIntervalTime(), flags));

	// m_timers owns Timer object in unique_ptr<Timer>.
	th->timers.insert(std::make_pair(event, timer));

	context->_getHandle()->callStateMonitor(context, [event](IContext* context, IStateMonitor* stateMonitor)
	{
		stateMonitor->onTimerStarted(context, event);
	});

	return S_OK;
}

/*static*/ VOID CALLBACK timerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired)
{
	CComPtr<TimerHandle::Timer> timer = (TimerHandle::Timer*)lpParameter;
	auto event = timer->event.p;
	auto timerClient = event->_getTimerClient();
	auto th = timerClient->_getHandle();
	th->timerCallback(timerClient, timer, event);
}

HRESULT TimerHandle::timerCallback(TimerClient* timerClient, Timer* timer, IEvent* event)
{
	{
		lock_t _lock(lock);

		auto it = timers.find(event);
		if(it != timers.end()) {
			if(event->_getIntervalTime() == 0) {
				// Delete timer except for interval timer.
				timers.erase(it);
			}
		} else {
			// The timer has been canceled.
			return S_FALSE;
		}
	}

	auto stateMachine = timer->context->_getStateMachine();
	switch(timer->timerType) {
	case TimerClient::TimerType::HandleEvent:
		HR_ASSERT_OK(stateMachine->handleEvent(timer->context, event));
		break;
	case TimerClient::TimerType::TriggerEvent:
		HR_ASSERT_OK(stateMachine->triggerEvent(timer->context, event));
		break;
	}
	return S_OK;
}

}
