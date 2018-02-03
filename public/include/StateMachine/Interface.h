#pragma once

#include <StateMachine/Unknown.h>

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

	// Data for IAsyncContext.
	struct AsyncData {
		std::thread thread;							// Worker thread.
		CHandle hReadyEvent;						// Event handle set when ready to handle IEvent.
		std::deque<CComPtr<IEvent>> eventQueue;		// FIFO of IEvent to be handled.
		std::recursive_mutex eventQueueLock;		// Lock to modify event queue.
	};

	// Returns nullptr(Async operation is not supported).
	virtual AsyncData* getAsyncData() { return nullptr; }

	CComPtr<IState> m_currentState;
	using lock_t = std::lock_guard<std::recursive_mutex>;
	virtual lock_t* getHandleEventLock() { return new lock_t(m_handleEventLock); }

protected:
	std::recursive_mutex m_handleEventLock;
};

class IAsyncContext : public IContext
{
public:
	virtual AsyncData* getAsyncData() { return &m_asyncData; }

protected:
	AsyncData m_asyncData;
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
