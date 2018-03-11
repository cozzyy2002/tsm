#include <StateMachine/stdafx.h>

#include <StateMachine/TimerClient.h>
#include <StateMachine/Assert.h>

#include "Handles.h"

HRESULT tsm::TimerClient::cancelEventTimer(IEvent* event)
{
	// Check if any timer has started by _triggerDelayedEvent() method.
	HR_ASSERT(m_hTimerQueue, E_ILLEGAL_METHOD_CALL);

	lock_t _lock(m_lock);

	auto it = m_timers.find(event);
	if(it != m_timers.end()) {
		// Remove timer and wait for currently running timer callback function.
		WIN32_ASSERT(DeleteTimerQueueTimer(m_hTimerQueue, it->second->hTimer, INVALID_HANDLE_VALUE));
		m_timers.erase(it);
		return S_OK;
	} else {
		// Timer specified might have expired already.
		return S_FALSE;
	}
}

HRESULT tsm::TimerClient::cancelAllEventTimers()
{
	lock_t _lock(m_lock);

	if(m_hTimerQueue) {
		// Cancel and delete all pending timers and delete timer queue.
		WIN32_EXPECT(DeleteTimerQueueEx(m_hTimerQueue, INVALID_HANDLE_VALUE));
		m_hTimerQueue = NULL;
		m_timers.clear();
	}

	return S_OK;
}

HRESULT tsm::TimerClient::_setEventTimer(TimerType timerType, IContext* context, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	// Ensure that timer is required.
	HR_ASSERT(event->_getTimerClient(), E_ILLEGAL_METHOD_CALL);

	// State machine should not call this method with event whose timer is created already.
	auto eh = event->_getHandle();
	HR_ASSERT(!eh->isTimerCreated, E_ILLEGAL_METHOD_CALL);
	eh->isTimerCreated = true;

	lock_t _lock(m_lock);

	if(!m_hTimerQueue) {
		// Create timer queue for first time.
		WIN32_ASSERT(m_hTimerQueue = CreateTimerQueue());
	}

	// Create timer object.
	std::unique_ptr<Timer> timer(new Timer());
	timer->timerType = timerType;
	timer->context = context;
	timer->event = event;

	// Create timer-queue timer and pass timer object as parameter for timer callback function.
	ULONG flags = 0;
	flags |= ((timer->timerType == TimerType::HandleEvent) ? WT_EXECUTELONGFUNCTION : 0);
	flags |= ((event->_getIntervalTime() == 0) ? WT_EXECUTEONLYONCE : 0);
	WIN32_ASSERT(CreateTimerQueueTimer(&timer->hTimer, m_hTimerQueue, timerCallback, timer.get(), event->_getDelayTime(), event->_getIntervalTime(), flags));

	// m_timers owns Timer object in unique_ptr<Timer>.
	m_timers.insert(std::make_pair(event, std::move(timer)));

	return S_OK;
}

/*static*/ VOID tsm::TimerClient::timerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	auto timer = (Timer*)lpParameter;
	auto event = timer->event.p;
	auto timerClient = event->_getTimerClient();

	// Timer object might be deleted when out of this method.
	std::unique_ptr<Timer> _timer;
	{
		lock_t _lock(timerClient->m_lock);

		auto it = timerClient->m_timers.find(event);
		if(it != timerClient->m_timers.end()) {
			if(event->_getIntervalTime() == 0) {
				// Delete timer except for interval timer.
				_timer = std::move(it->second);
				timerClient->m_timers.erase(it);
			}
		} else {
			HR_EXPECT(!_T("Callback of canceled timer is called."), E_UNEXPECTED);
			return;
		}
	}

	auto stateMachine = timer->context->_getStateMachine();
	switch(timer->timerType) {
	case TimerType::HandleEvent:
		HR_EXPECT_OK(stateMachine->handleEvent(timer->context, event));
		break;
	case TimerType::TriggerEvent:
		HR_EXPECT_OK(stateMachine->triggerEvent(timer->context, event));
		break;
	}
}
