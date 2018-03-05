#include <StateMachine/stdafx.h>

#include <StateMachine/TimerClient.h>
#include <StateMachine/Assert.h>

HRESULT tsm::TimerClient::_triggerDelayedEvent(IContext * context, DWORD timeout, IEvent * event, ITimerClient::Timer ** ppTimer)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);
	HR_ASSERT(event, E_INVALIDARG);

	IContext::lock_t _lock(m_lock);

	if(!m_hTimerQueue) {
		HANDLE hTimerQueue;
		WIN32_ASSERT(hTimerQueue = CreateTimerQueue());
		m_hTimerQueue.Attach(m_hTimerQueue);
	}
	auto timer = new ITimerClient::Timer(context, this, event);
	m_timers.insert(std::make_pair(timer, timer));
	HANDLE hTimer;
	WIN32_ASSERT(CreateTimerQueueTimer(&hTimer, m_hTimerQueue, timerCallback, timer, timeout, 0, 0));
	timer->hTimer.Attach(hTimer);
	*ppTimer = timer;
	return S_OK;
}

HRESULT tsm::TimerClient::cancelDelayedEvent(ITimerClient::Timer * pTimer)
{
	IContext::lock_t _lock(m_lock);

	return E_NOTIMPL;
}

HRESULT tsm::TimerClient::stopAllTimers()
{
	IContext::lock_t _lock(m_lock);

	for(auto& item : m_timers) {
		auto timer = item.first;
	}
	m_timers.clear();

	return E_NOTIMPL;
}

/*static*/ VOID tsm::TimerClient::timerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	auto timer = (ITimerClient::Timer*)lpParameter;
	auto client = (TimerClient*)timer->client;

	IContext::lock_t _lock(client->m_lock);

	auto it = client->m_timers.find(timer);
	if(it != client->m_timers.end()) {
		timer->context->_getStateMachine()->triggerEvent(timer->context, timer->event);
		client->m_timers.erase(it);
	} else {
		// Expired timer is not exist in m_timers.
	}
}
