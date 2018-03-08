#pragma once

#include "Interface.h"

#include <map>

namespace tsm {

class TimerClient : public ITimerClient
{
public:
	TimerClient() : m_hTimerQueue(NULL) {}

	virtual HRESULT _handleDelayedEvent(IContext* context, IEvent* event, DWORD timeout, ITimerClient::Timer** ppTimer) override;
	virtual HRESULT _triggerDelayedEvent(IContext* context, IEvent* event, DWORD timeout, ITimerClient::Timer** ppTimer) override;
	virtual HRESULT cancelDelayedEvent(ITimerClient::Timer* pTimer) override;
	virtual HRESULT stopAllTimers() override;

protected:
	std::map<Timer*, std::unique_ptr<Timer>> m_timers;

	// Note: Closing handle of timer queue is not necessary.
	HANDLE m_hTimerQueue;
	lock_object_t m_lock;

	HRESULT setDelayedEvent(Timer* timer, DWORD timeout, ITimerClient::Timer** ppTimer);
	static VOID CALLBACK timerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired);
};

}
