#include <StateMachine/stdafx.h>

#include <StateMachine/TimerClient.h>
#include <StateMachine/Assert.h>

HRESULT tsm::TimerClient::_triggerDelayedEvent(IContext * context, DWORD timeout, IEvent * event, ITimerClient::Timer ** ppTimer)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);
	HR_ASSERT(event, E_INVALIDARG);

	lock_t _lock(m_lock);

	if(!m_hTimerQueue) {
		// Create timer queue for first time.
		WIN32_ASSERT(m_hTimerQueue = CreateTimerQueue());
	}
	auto timer = new ITimerClient::Timer(context, this, event);
	m_timers.insert(std::make_pair(timer, timer));
	HANDLE hTimer;
	WIN32_ASSERT(CreateTimerQueueTimer(&hTimer, m_hTimerQueue, timerCallback, timer, timeout, 0, 0));
	timer->hTimer.Attach(hTimer);
	if(ppTimer) {
		*ppTimer = timer;
	}
	return S_OK;
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
		for(auto& item : m_timers) {
			auto timer = item.first;
			// Remove timer and wait for currently running timer callback function.
			WIN32_EXPECT(DeleteTimerQueueTimer(m_hTimerQueue, timer->hTimer, INVALID_HANDLE_VALUE));
		}
		m_timers.clear();

		WIN32_EXPECT(DeleteTimerQueue(m_hTimerQueue));
		m_hTimerQueue = NULL;
	}

	return S_OK;
}

/*static*/ VOID tsm::TimerClient::timerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	auto timer = (ITimerClient::Timer*)lpParameter;
	auto client = (TimerClient*)timer->client;

	lock_t _lock(client->m_lock);

	auto it = client->m_timers.find(timer);
	if(it != client->m_timers.end()) {
		timer->context->_getStateMachine()->triggerEvent(timer->context, timer->event);
		client->m_timers.erase(it);
	} else {
		// Expired timer might have been canceled.
	}
}
