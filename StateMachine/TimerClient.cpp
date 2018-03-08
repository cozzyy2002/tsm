#include <StateMachine/stdafx.h>

#include <StateMachine/TimerClient.h>
#include <StateMachine/Assert.h>

HRESULT tsm::TimerClient::_handleDelayedEvent(IContext * context, IEvent * event, DWORD timeout, ITimerClient::Timer ** ppTimer)
{
	auto timer = new Timer(Timer::Type::HandleEvent, context, this, event);
	return HR_EXPECT_OK(setDelayedEvent(timer, timeout, ppTimer));
}

HRESULT tsm::TimerClient::_triggerDelayedEvent(IContext * context, IEvent* event, DWORD timeout, ITimerClient::Timer ** ppTimer)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	// Async operation is required.
	HR_ASSERT(context->isAsync(), E_NOTIMPL);

	auto timer = new Timer(Timer::Type::TriggerEvent, context, this, event);
	return HR_EXPECT_OK(setDelayedEvent(timer, timeout, ppTimer));
}

HRESULT tsm::TimerClient::cancelDelayedEvent(ITimerClient::Timer * timer)
{
	// Check if any timer has started by _triggerDelayedEvent() method.
	HR_ASSERT(m_hTimerQueue, E_ILLEGAL_METHOD_CALL);

	lock_t _lock(m_lock);

	auto it = m_timers.find(timer);
	if(it != m_timers.end()) {
		// Remove timer and wait for currently running timer callback function.
		WIN32_ASSERT(DeleteTimerQueueTimer(m_hTimerQueue, timer->hTimer, INVALID_HANDLE_VALUE));
		m_timers.erase(it);
		return S_OK;
	} else {
		// Timer specified might have expired already.
		return S_FALSE;
	}
}

HRESULT tsm::TimerClient::stopAllTimers()
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

HRESULT tsm::TimerClient::setDelayedEvent(ITimerClient::Timer* timer, DWORD timeout, ITimerClient::Timer ** ppTimer)
{
	// Ensure to release object on error.
	std::unique_ptr<Timer> _timer(timer);

	lock_t _lock(m_lock);

	if(!m_hTimerQueue) {
		// Create timer queue for first time.
		WIN32_ASSERT(m_hTimerQueue = CreateTimerQueue());
	}

	// m_timers owns Timer object in unique_ptr<Timer>.
	m_timers.insert(std::make_pair(timer, std::move(_timer)));
	WIN32_ASSERT(CreateTimerQueueTimer(&timer->hTimer, m_hTimerQueue, timerCallback, timer, timeout, 0, 0));
	if(ppTimer) {
		*ppTimer = timer;
	}
	return S_OK;
}

/*static*/ VOID tsm::TimerClient::timerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	std::unique_ptr<Timer> timer((ITimerClient::Timer*)lpParameter);
	auto client = timer->client;

	bool timerExists = false;
	{
		lock_t _lock(client->m_lock);

		auto it = client->m_timers.find(timer.get());
		if(it != client->m_timers.end()) {
			it->second.release();
			client->m_timers.erase(it);
			timerExists = true;
		} else {
			// Expired timer might have been canceled.
		}
	}

	if(timerExists) {
		auto stateMachine = timer->context->_getStateMachine();
		switch(timer->type) {
		case Timer::Type::HandleEvent:
			HR_EXPECT_OK(stateMachine->handleEvent(timer->context, timer->event));
			break;
		case Timer::Type::TriggerEvent:
			HR_EXPECT_OK(stateMachine->triggerEvent(timer->context, timer->event));
			break;
		}
	}
}
