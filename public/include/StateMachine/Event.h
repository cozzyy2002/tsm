#pragma once

#include "Interface.h"

namespace tsm {

template<class C = IContext>
class Event : public IEvent
{
public:
	// Default event priority.
	// This value is used by AsyncContext::triggerEvent().
	// Context::triggerEvent() is not implemented.
	static const int DefaultPriority = 0;

	Event(int priority = DefaultPriority)
		: m_priority(priority), m_timerClient(nullptr), m_delayTime(0), m_intervalTime(0), m_timeoutCount(-1)
		, m_handle(HandleFactory<IEvent, EventHandle>::create(this)) {}
	virtual ~Event() {}

#pragma region Implementation of IState that call methods of sub class.
	HRESULT _preHandle(IContext* context) override {
		return preHandle((C*)context);
	}
	HRESULT _postHandle(IContext* context, HRESULT hr) override {
		return postHandle((C*)context, hr);
	}
	int _comparePriority(IEvent* other) const override {
		return comparePriority((Event*)other);
	}
#pragma endregion

#pragma region Methods to be implemented by sub class.
	virtual HRESULT preHandle(C* context) { return S_OK; }
	virtual HRESULT postHandle(C* context, HRESULT hr) { return hr; }
	virtual int comparePriority(Event* other) const { return (this->m_priority - other->m_priority); }
#pragma endregion

	virtual int _getPriority() const override { return m_priority; }
	virtual DWORD _getDelayTime() const override { return m_delayTime; }
	virtual DWORD _getIntervalTime() const override { return m_intervalTime; }
	virtual ITimerClient* _getTimerClient() const override { return m_timerClient; }
	virtual int _getTimeoutCount() const override { return m_timeoutCount; }
protected:
	virtual void _setTimeoutCount(int count) override { m_timeoutCount = count; }

public:
	// Set delay time to one-shot timer.
	void setDelayTimer(ITimerOwner* timerOwner, DWORD delayTime) { setTimer(timerOwner, delayTime, 0); }
	// Set interval time to interval timer.
	void setIntervaleTimer(ITimerOwner* timerOwner, DWORD intervalTime) { setTimer(timerOwner, 0, intervalTime); }
	// Set delay time and interval time.
	void setTimer(ITimerOwner* timerOwner, DWORD delayTime, DWORD intervalTime = 0) {
		setTimer(timerOwner->_getTimerClient(), delayTime, intervalTime);
	}
	void setTimer(ITimerClient* timerClient, DWORD delayTime, DWORD intervalTime = 0) {
		m_delayTime = delayTime;
		m_intervalTime = intervalTime;
		m_timerClient = timerClient;
	}
	HRESULT cancelTimer(int timeout = 0) {
		return m_timerClient ? m_timerClient->cancelEventTimer(this, timeout) : E_ILLEGAL_METHOD_CALL;
	}

	virtual EventHandle* _getHandle() override { return m_handle.get(); }

protected:
	std::unique_ptr<EventHandle, HandleFactory<IEvent, EventHandle>> m_handle;
	int m_priority;
	DWORD m_delayTime;
	DWORD m_intervalTime;
	ITimerClient* m_timerClient;
	int m_timeoutCount;
};

}
