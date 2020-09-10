#pragma once

#include <StateMachine/Interface.h>

#include <vector>

namespace tsm {

struct TimerHandle;

class TimerClient : public HandleOwner<TimerClient, TimerHandle>
{
public:
	HRESULT cancelEventTimer(IEvent* event, int timeout = 0);
	HRESULT cancelAllEventTimers(int timeout = 0);
	std::vector<CComPtr<IEvent>> getPendingEvents();

	enum class TimerType {
		None,			// Event is handled ASAP. This value is not used.
		HandleEvent,	// Call StateMachine::handleEvent() when the timer expires.
		TriggerEvent,	// Call StateMachine::triggerEvent() when the timer expires.
	};
	HRESULT _setEventTimer(TimerType timerType, IContext* context, IEvent* event);
};

}
