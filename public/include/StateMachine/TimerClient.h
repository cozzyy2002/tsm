#pragma once

#include <StateMachine/Interface.h>

namespace tsm {

struct TimerHandle;

class TimerClient : public HandleOwner<TimerClient, TimerHandle>
{
public:
	HRESULT cancelEventTimer(IEvent* event);
	HRESULT cancelAllEventTimers();

	enum class TimerType {
		None,			// Event is handled ASAP. This value is not used.
		HandleEvent,	// Call StateMachine::handleEvent() when timer is elapsed.
		TriggerEvent,	// Call StateMachine::triggerEvent() when timer is elapsed.
	};
	HRESULT _setEventTimer(TimerType timerType, IContext* context, IEvent* event);

protected:
	static VOID CALLBACK timerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired);
};

}
