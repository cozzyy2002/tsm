#pragma once

#include "Unknown.h"

#include <deque>
#include <memory>
#include <mutex>
#include <thread>

namespace tsm {

class IEvent;
class IState;

class IContext
{
public:
	virtual ~IContext() {}

	// Data for IAsyncContext used to perform async operation.
	struct AsyncData {
		std::thread thread;							// Worker thread.
		CHandle hEventReady;						// Event handle set when ready to handle IEvent.
		std::deque<CComPtr<IEvent>> eventQueue;		// FIFO of IEvent to be handled.
		CHandle hEventAvailable;					// Event handle set when event is queued.
		CHandle hEventShutdown;						// Event handle set to shutdown the state machine.
		std::recursive_mutex eventQueueLock;		// Lock to modify event queue.
	};

	// Returns nullptr(Async operation is not supported).
	virtual AsyncData* getAsyncData() = 0;

	using lock_t = std::lock_guard<std::recursive_mutex>;
	virtual lock_t* getHandleEventLock() { return new lock_t(m_handleEventLock); }

	CComPtr<IState> m_currentState;

protected:
	std::recursive_mutex m_handleEventLock;
};

class IEvent : public Unknown
{
public:
	virtual ~IEvent() {}
};

class IState : public Unknown
{
public:
	IState(IState* masterState) : m_masterState(masterState) {}
	virtual ~IState() {}

#pragma region Methods to be called by StateMachine.
	virtual HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) = 0;
	virtual HRESULT _entry(IContext* context, IEvent* event, IState* previousState) = 0;
	virtual HRESULT _exit(IContext* context, IEvent* event, IState* nextState) = 0;
#pragma endregion

	CComPtr<IState> m_subState;
	IState* m_masterState;
};

class IStateMachine
{
public:
	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event) = 0;
	virtual HRESULT shutdown(IContext* context) = 0;
	virtual HRESULT triggerEvent(IContext* context, IEvent* event) = 0;
	virtual HRESULT handleEvent(IContext* context, IEvent* event) = 0;
};

extern IStateMachine* createStateMachine();
}
