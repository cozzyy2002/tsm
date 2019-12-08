#pragma once

#include <StateMachine/Interface.h>
#include <StateMachine/TimerClient.h>
#include <memory>
#include <map>
#include <deque>
#include <mutex>

#define MEMORY_WEIGHT_MBYTE 100
#if !defined(MEMORY_WEIGHT_MBYTE)
#defnie MEMORY_WEIGHT_MBYTE 0
#endif
#if MEMORY_WEIGHT_MBYTE
#include "MemoryWeight.h"
#endif

namespace tsm {

using lock_object_t = std::recursive_mutex;
using lock_t = std::lock_guard<lock_object_t>;

struct EventHandle
#if MEMORY_WEIGHT_MBYTE
	: public MemoryWeight
#endif
{
	EventHandle(IEvent*);

	bool isTimerCreated;
};

struct StateHandle
#if MEMORY_WEIGHT_MBYTE
	: public MemoryWeight
#endif
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
		CHandle hEventReady;						// Event handle set when ready to handle IEvent(entry() of Initial state completes).

		// For AsyncStateMachine
		CHandle hEventAvailable;					// Event handle set when IEvent is queued.
		CHandle hEventShutdown;						// Event handle set to shutdown the state machine.
		CHandle hWorkerThread;						// Handle of worker thread.

		// For WindowProcStateMachine
		HWND hWnd;		// Window handle to which state machine message is sent.
		UINT msg;		// Message ID used by state machine.
		WNDPROC appWndProc;	// Window procedure of app.

		HRESULT queueEvent(IEvent* event);
	};

	AsyncData* asyncData;

	lock_t* getHandleEventLock() { return new lock_t(_handleEventLock); }
	void callStateMonitor(IContext* context, std::function<void(IContext* context, IStateMonitor* stateMonitor)> caller);

protected:
	std::unique_ptr<AsyncData> _asyncData;
	lock_object_t _handleEventLock;
};

struct TimerHandle
{
	TimerHandle(TimerClient*);

	struct Timer : public Unknown {
		TimerClient::TimerType timerType;
		IContext* context;
		CComPtr<IEvent> event;
		HANDLE hTimer;				// Handle of timer-queue timer.
	};

	std::map<IEvent*, CComPtr<Timer>> timers;

	// Note: Closing handle of timer queue is not necessary.
	HANDLE hTimerQueue;
	lock_object_t lock;

	HRESULT timerCallback(TimerClient* timerClient, Timer* timer, IEvent* event);
};

}
