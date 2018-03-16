#pragma once

#include <StateMachine/Interface.h>
#include <StateMachine/TimerClient.h>
#include <memory>
#include <map>
#include <deque>

namespace tsm {

struct EventHandle
{
	EventHandle(IEvent*);

	bool isTimerCreated;
};

struct StateHandle
{
	StateHandle(IState*);

	bool entryCalled;
};

struct ContextHandle
{
	ContextHandle(IContext*);

	// Data for IAsyncContext used to perform async operation.
	struct AsyncData {
		std::deque<CComPtr<IEvent>> eventQueue;		// FIFO of IEvent to be handled.
		std::recursive_mutex eventQueueLock;		// Lock to modify IEvent queue.
		CHandle hEventAvailable;					// Event handle set when IEvent is queued.
		CHandle hEventShutdown;						// Event handle set to shutdown the state machine.
		CHandle hEventReady;						// Event handle set when ready to handle IEvent(entry() of Initial state completes).
		CHandle hWorkerThread;						// Handle of worker thread.
	};

	AsyncData* asyncData;

protected:
	friend class HandleOwner<IContext, ContextHandle>;

	std::unique_ptr<AsyncData> _asyncData;
};

struct TimerHandle
{
	TimerHandle(TimerClient*);

	struct Timer {
		TimerClient::TimerType timerType;
		IContext* context;
		CComPtr<IEvent> event;
		HANDLE hTimer;				// Handle of timer-queue timer.
	};

	std::map<IEvent*, std::unique_ptr<Timer>> timers;

	// Note: Closing handle of timer queue is not necessary.
	HANDLE hTimerQueue;
	lock_object_t lock;

	HRESULT timerCallback(TimerClient* timerClient, Timer* timer, IEvent* event);
};

}
