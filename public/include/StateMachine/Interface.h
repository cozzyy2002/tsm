#pragma once

#include "Unknown.h"

#include <deque>
#include <map>
#include <utility>
#include <memory>
#include <mutex>

namespace tsm {

class IEvent;
class IState;
class IStateMachine;
class IStateMonitor;

class IContext
{
public:
	virtual ~IContext() {}

	virtual bool isAsync() const = 0;

	// Data for IAsyncContext used to perform async operation.
	struct AsyncData {
		std::deque<CComPtr<IEvent>> eventQueue;		// FIFO of (priority, IEvent to be handled).
		std::recursive_mutex eventQueueLock;		// Lock to modify IEvent queue.
		CHandle hEventAvailable;					// Event handle set when IEvent is queued.
		CHandle hEventShutdown;						// Event handle set to shutdown the state machine.
		CHandle hEventReady;						// Event handle set when ready to handle IEvent(entry() of Initial state completes).
		CHandle hWorkerThread;						// Handle of worker thread.
	};

	virtual IStateMachine* _getStateMachine() = 0;
	virtual IState* _getCurrentState() = 0;
	virtual void _setCurrentState(IState* state) = 0;

	using OnAssertFailed = void(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);
	static OnAssertFailed* onAssertFailedProc;

	// Returns nullptr(Async operation is not supported).
	virtual AsyncData* _getAsyncData() = 0;

	using lock_object_t = std::recursive_mutex;
	using lock_t = std::lock_guard<lock_object_t>;
	virtual lock_t* _getHandleEventLock() = 0;

	virtual IStateMonitor* _getStateMonitor() = 0;
};

class IEvent : public Unknown
{
public:
	virtual ~IEvent() {}

	virtual int _getPriority() const = 0;
};

class IState : public Unknown
{
public:
	virtual ~IState() {}

#pragma region Methods to be called by StateMachine.
	virtual HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) = 0;
	virtual HRESULT _entry(IContext* context, IEvent* event, IState* previousState) = 0;
	virtual HRESULT _exit(IContext* context, IEvent* event, IState* nextState) = 0;

	virtual IState* _getMasterState() const = 0;
	virtual void _setMasterState(IState* state) = 0;
	virtual bool _hasEntryCalled() const = 0;
#pragma endregion
};

class ITimerClient
{
public:
	struct Timer {
		ITimerClient* client;
		HANDLE hTimer;
		CComPtr<IEvent> event;
	};

	using timers_t = std::map<Timer*, Timer>;
	virtual timers_t& getTimers() = 0;
	virtual HANDLE getTimerQueue() = 0;
	virtual void setTimerQueue(HANDLE hTimerQueue) = 0;
};

class IStateMachine
{
public:
	static IStateMachine* create(IContext* context);

	virtual ~IStateMachine() {}

	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event) = 0;
	virtual HRESULT shutdown(IContext* context, DWORD timeout) = 0;
	virtual HRESULT triggerEvent(IContext* context, IEvent* event, int priority) = 0;
	virtual HRESULT triggerDelayedEvent(ITimerClient* client, ITimerClient::Timer* pTimer, DWORD timeout, IEvent* event) = 0;
	virtual HRESULT cancelDelayedEvent(ITimerClient* client, ITimerClient::Timer* pTimer) = 0;
	virtual HRESULT handleEvent(IContext* context, IEvent* event) = 0;
	virtual HRESULT waitReady(IContext* context, DWORD timeout) = 0;
};

// Monitor interface
class IStateMonitor
{
public:
	virtual void onIdle(IContext* context) = 0;
	virtual void onStateChanged(IContext* context, IEvent* event, IState* previous, IState* next) = 0;
	virtual void onWorkerThreadExit(IContext* context, HRESULT exitCode) = 0;
};

}
