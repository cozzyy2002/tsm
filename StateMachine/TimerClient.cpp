#include <StateMachine/stdafx.h>

#include <StateMachine/TimerClient.h>
#include <StateMachine/Assert.h>

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
	WIN32_ASSERT(CreateTimerQueueTimer(&timer->hTimer, m_hTimerQueue, timerCallback, timer.get(), event->_getDelayTime(), event->_getIntervalTime(), 0));

	// m_timers owns Timer object in unique_ptr<Timer>.
	m_timers.insert(std::make_pair(event, std::move(timer)));

	return S_OK;
}

/*static*/ VOID tsm::TimerClient::timerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	std::unique_ptr<Timer> timer((Timer*)lpParameter);
	auto timerClient = timer->event->_getTimerClient();

	bool timerExists = false;
	{
		lock_t _lock(timerClient->m_lock);

		auto it = timerClient->m_timers.find(timer->event);
		if(it != timerClient->m_timers.end()) {
			it->second.release();
			timerClient->m_timers.erase(it);
			timerExists = true;
		} else {
			// Expired timer might have been canceled.
		}
	}

	if(timerExists) {
		auto stateMachine = timer->context->_getStateMachine();
		switch(timer->timerType) {
		case TimerType::HandleEvent:
			HR_EXPECT_OK(stateMachine->handleEvent(timer->context, timer->event));
			break;
		case TimerType::TriggerEvent:
			timer->event->_setTimerClient(nullptr);
			HR_EXPECT_OK(stateMachine->triggerEvent(timer->context, timer->event));
			timer->event->_setTimerClient(timerClient);
			break;
		}
	}
}
