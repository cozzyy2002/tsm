#pragma once

#include "Interface.h"

namespace tsm {

class Event : public IEvent
{
public:
	// Default event priority.
	// This value is used by AsyncContext::triggerEvent().
	// Context::triggerEvent() is not implemented.
	static const int DefaultPriority = 0;

	Event(int priority = DefaultPriority)
		: m_priority(priority), m_timerClient(nullptr) {}
	virtual ~Event() {}

	virtual int _getPriority() const override { return m_priority; }
	virtual DWORD _getDelayTime() const override { return m_delayTime; }
	virtual DWORD _getIntervalTime() const override { return m_intervalTime; }
	virtual TimerClient* _getTimerClient() const override { return m_timerClient; }

	void setTimer(TimerClient* timerClient, DWORD delayTime, DWORD intervalTime = 0) {
		m_delayTime = delayTime;
		m_intervalTime = intervalTime;
		m_timerClient = timerClient;
	}

protected:
	int m_priority;
	DWORD m_delayTime;
	DWORD m_intervalTime;
	TimerClient* m_timerClient;
};

}
