#pragma once

#include <StateMachine/Interface.h>

namespace tsm {

struct TimerHandle;

class TimerClient : public ITimerClient, public HandleOwner<TimerClient, TimerHandle>
{
public:
	HRESULT cancelEventTimer(IEvent* event);
	HRESULT cancelAllEventTimers();

	HRESULT _setEventTimer(TimerType timerType, IContext* context, IEvent* event);

protected:
	static VOID CALLBACK timerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired);
};

}
