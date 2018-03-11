#pragma once

#include <StateMachine/Interface.h>
#include <memory>
#include <map>

namespace tsm {

struct EventHandle
{
	bool isTimerCreated;
};

struct StateHandle
{
};

struct ContextHandle
{
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
	struct Timer {
		ITimerClient::TimerType timerType;
		IContext* context;
		CComPtr<IEvent> event;
		HANDLE hTimer;				// Handle of timer-queue timer.
	};

	std::map<IEvent*, std::unique_ptr<Timer>> timers;

	// Note: Closing handle of timer queue is not necessary.
	HANDLE hTimerQueue;
	lock_object_t lock;
};

}
