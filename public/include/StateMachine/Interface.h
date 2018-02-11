#pragma once

#include "Unknown.h"

#include <deque>
#include <memory>
#include <mutex>
#include <thread>

namespace tsm {

class IEvent;
class IState;
class IStateMachine;

class IContext
{
public:
	virtual ~IContext() {}

	// Data for IAsyncContext used to perform async operation.
	struct AsyncData {
		std::deque<CComPtr<IEvent>> eventQueue;		// FIFO of IEvent to be handled.
		std::recursive_mutex eventQueueLock;		// Lock to modify IEvent queue.
		CHandle hEventAvailable;					// Event handle set when IEvent is queued.
		CHandle hEventShutdown;						// Event handle set to shutdown the state machine.
		CHandle hEventReady;						// Event handle set when ready to handle IEvent(entry() of Initial state completes).
		CHandle hWorkerThread;						// Handle of worker thread.
	};

	virtual IStateMachine* _getStateMachine() = 0;
	virtual IState* _getCurrentState() = 0;
	virtual void _setCurrentState(IState* state) = 0;

	// Returns nullptr(Async operation is not supported).
	virtual AsyncData* _getAsyncData() = 0;

	using lock_object_t = std::recursive_mutex;
	using lock_t = std::lock_guard<lock_object_t>;
	virtual lock_t* _getHandleEventLock() = 0;
};

class IEvent : public Unknown
{
public:
	virtual ~IEvent() {}
};

class IState : public Unknown
{
public:
	virtual ~IState() {}

#pragma region Methods to be called by StateMachine.
	virtual HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) = 0;
	virtual HRESULT _entry(IContext* context, IEvent* event, IState* previousState) = 0;
	virtual HRESULT _exit(IContext* context, IEvent* event, IState* nextState) = 0;
#pragma endregion

	virtual IState* _getMasterState() = 0;
	virtual void _setMasterState(IState* state) = 0;
	virtual bool _hasEntryCalled() = 0;
	virtual void _setEntryCalled(bool value) = 0;
};

class IStateMachine
{
public:
	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event) = 0;
	virtual HRESULT shutdown(IContext* context, DWORD timeout) = 0;
	virtual HRESULT triggerEvent(IContext* context, IEvent* event) = 0;
	virtual HRESULT handleEvent(IContext* context, IEvent* event) = 0;
	virtual HRESULT waitReady(IContext* context, DWORD timeout) = 0;
};

extern IStateMachine* createStateMachine();
}
