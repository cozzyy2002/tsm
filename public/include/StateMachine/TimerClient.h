#pragma once

#include "Interface.h"

namespace tsm {

class TimerClient : public ITimerClient
{
public:
	virtual timers_t& getTimers() override { return m_timers; }
	virtual HANDLE getTimerQueue() override { return m_hTimerQueue; }
	virtual void setTimerQueue(HANDLE hTimerQueue) override { m_hTimerQueue.Attach(hTimerQueue); }

protected:
	timers_t m_timers;
	CHandle m_hTimerQueue;
};

}
