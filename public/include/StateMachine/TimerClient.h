#pragma once

#include <StateMachine/Interface.h>

#include <map>

namespace tsm {

class TimerClient
{
public:
	enum class TimerType {
		None,			// Event is handled ASAP. This value is not used.
		HandleEvent,	// Call StateMachine::handleEvent() when timer is elapsed.
		TriggerEvent,	// Call StateMachine::triggerEvent() when timer is elapsed.
	};

	struct Timer {
		TimerType timerType;
		IContext* context;
		CComPtr<IEvent> event;
		HANDLE hTimer;				// Handle of timer-queue timer.
	};

	TimerClient() : m_hTimerQueue(NULL) {}

	HRESULT _setEventTimer(TimerType timerType, IContext* context, IEvent* event);
	HRESULT cancelEventTimer(IEvent* event);
	HRESULT cancelAllEventTimers();

protected:
	std::map<IEvent*, std::unique_ptr<Timer>> m_timers;

	// Note: Closing handle of timer queue is not necessary.
	HANDLE m_hTimerQueue;
	lock_object_t m_lock;

	static VOID CALLBACK timerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired);
};

}
