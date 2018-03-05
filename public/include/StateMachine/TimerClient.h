#pragma once

#include "Interface.h"

namespace tsm {

class TimerClient : public ITimerClient
{
public:
	virtual HRESULT _triggerDelayedEvent(IContext* context, DWORD timeout, IEvent* event, ITimerClient::Timer** ppTimer) override;
	virtual HRESULT cancelDelayedEvent(ITimerClient::Timer* pTimer) override;
	virtual HRESULT stopAllTimers() override;

protected:
	timers_t m_timers;
	CHandle m_hTimerQueue;
	IContext::lock_object_t m_lock;

	static VOID CALLBACK timerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired);
};

}
