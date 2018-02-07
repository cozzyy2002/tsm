#include <StateMachine/stdafx.h>

#include <StateMachine/State.h>
#include "StateMachine.h"

#define HR_ASSERT_OK(exp) do { auto _hr(HR_EXPECT_OK(exp)); if(FAILED(_hr)) return _hr; } while(false)
#define HR_EXPECT_OK(exp) (exp)

namespace tsm {

IStateMachine* createStateMachine()
{
	return new StateMachine();
}

StateMachine::StateMachine()
{
}

/*
 * State class for StateMachine::startup() method.
 */
class StartupState : public State<>
{
public:
	StartupState(IState* initialState) : m_initialState(initialState) {}

	// Sets initial state as next state regardless of event.
	HRESULT handleEvent(IContext* context, IEvent* event, IState** nextState) {
		*nextState = m_initialState;
		return S_OK;
	}

protected:
	CComPtr<IState> m_initialState;
};

HRESULT StateMachine::setup(IContext* context, IState * initialState, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IState> _initialState(initialState);
	CComPtr<IEvent> _event(event);

	// Check if setup() has not been called.
	HR_ASSERT_OK(FAILED(setupCompleted(context)) ? S_OK : E_ILLEGAL_METHOD_CALL);

	// Set StartupState object as current state.
	context->m_currentState = new StartupState(initialState);

	auto asyncData = context->getAsyncData();
	if(asyncData) {
		// Setup event queue.
		asyncData->eventQueue.clear();
		asyncData->hEventAvailable.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
		triggerEvent(context, event);

		// Setup worker thread in which event handling is performed.
		// entry() of initial state will be called in the thread.
		asyncData->hEventReady.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
		asyncData->hEventShutdown.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
		asyncData->thread = std::thread([this, context] { doWorkerThread(context); });
	} else {
		// Call entry() of initial state in the caller thread.
		HR_ASSERT_OK(handleEvent(context, event));
	}

	return S_OK;
}

/*
 * Shutdown state machine.
 *
 * This method can be called even if setup() has been called.
 */
HRESULT StateMachine::shutdown(IContext* context)
{
	auto asyncData = context->getAsyncData();
	if(asyncData && asyncData->hEventShutdown) {
		// Signal worker thread to shutdown and wait for it to terminate.
		SetEvent(asyncData->hEventShutdown);
		auto& thread = asyncData->thread;
		if(thread.joinable()) {
			thread.join();
		}
	}

	context->m_currentState.Release();

	return S_OK;
}

HRESULT StateMachine::triggerEvent(IContext * context, IEvent * event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	HR_ASSERT_OK(setupCompleted(context));

	auto asyncData = context->getAsyncData();
	if(asyncData) {
		// Add the event to event queue and signal that event is available.
		{
			IContext::lock_t _lock(asyncData->eventQueueLock);
			asyncData->eventQueue.push_back(event);
		}
		SetEvent(asyncData->hEventAvailable);
		return S_OK;
	} else {
		// Async operation is not supported.
		return E_ILLEGAL_METHOD_CALL;
	}
}

HRESULT StateMachine::handleEvent(IContext* context, IEvent * event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	HR_ASSERT_OK(setupCompleted(context));

	std::unique_ptr<IContext::lock_t> _lock(context->getHandleEventLock());

	CComPtr<IEvent> e(event);
	IState* nextState = nullptr;
	HRESULT hr = HR_EXPECT_OK(context->m_currentState->_handleEvent(context, event, &nextState));
	CComPtr<IState> _nextState(nextState);
	if(SUCCEEDED(hr) && nextState) {
		HR_ASSERT_OK(context->m_currentState->_exit(context, event, nextState));
		CComPtr<IState> previousState(context->m_currentState);
		context->m_currentState = nextState;
		HR_ASSERT_OK(context->m_currentState->_entry(context, event, previousState));
	}
	return hr;
}

HRESULT StateMachine::waitReady(IContext * context, DWORD timeout)
{
	auto asyncData = context->getAsyncData();
	if(asyncData) {
		DWORD wait = WaitForSingleObject(asyncData->hEventReady, timeout);
		return HR_EXPECT_OK(checkWaitResult(wait));
	} else {
		return S_OK;
	}
}

HRESULT StateMachine::doWorkerThread(IContext* context)
{
	auto asyncData = context->getAsyncData();
	HANDLE hEvents[] = { asyncData->hEventAvailable, asyncData->hEventShutdown };
	static const DWORD eventsCount = ARRAYSIZE(hEvents);

	auto first = true;
	do {
		// Wait for event to be queued.
		DWORD wait = WaitForMultipleObjects(eventsCount, hEvents, FALSE, INFINITE);
		if(wait < eventsCount) ResetEvent(hEvents[wait]);
		HR_ASSERT_OK(checkWaitResult(wait, eventsCount));
		if(wait == (WAIT_OBJECT_0 + 1)) break;
		do {
			CComPtr<IEvent> event;
			{
				// Fetch event from the queue.
				IContext::lock_t _lock(asyncData->eventQueueLock);
				if(asyncData->eventQueue.empty()) break;
				event = asyncData->eventQueue.front();
				asyncData->eventQueue.pop_front();
			}

			// Handle the event.
			HR_ASSERT_OK(handleEvent(context, event));
			if(first) {
				// Set ready event after first event handled(AsyncContext::setup() is completed).
				first = false;
				SetEvent(asyncData->hEventReady);
			}
		} while(true);
	} while(true);
	return S_OK;
}

HRESULT StateMachine::setupCompleted(IContext* context) const
{
	return context->m_currentState ? S_OK : E_ILLEGAL_METHOD_CALL;
}

// Check return value of WaitForSingleObject() and WaitForMultipleObjects().
HRESULT StateMachine::checkWaitResult(DWORD wait, DWORD eventCount /*= 1*/) const
{
	switch(wait) {
	case WAIT_TIMEOUT:
		return HRESULT_FROM_WIN32(WAIT_TIMEOUT);
	case WAIT_FAILED:
		return HRESULT_FROM_WIN32(GetLastError());
	default:
		if((WAIT_OBJECT_0 <= wait) && (wait < (WAIT_OBJECT_0 + eventCount))) {
			// Wait succeeded.
			return S_OK;
		} else {
			// Unknown return value.
			return E_UNEXPECTED;
		}
	}
}

}