#include <StateMachine/stdafx.h>

#include <StateMachine/State.h>
#include <StateMachine/Assert.h>
#include "StateMachine.h"

HRESULT checkHResult(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	if(FAILED(hr)) {
		_tprintf_s(_T("'%s' failed: HRESULT=0x%08x at:\n%s:%d\n"), exp, hr, sourceFile, line);
	}
	return hr;
}

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
	HR_ASSERT(FAILED(setupCompleted(context)), E_ILLEGAL_METHOD_CALL);

	// Set StartupState object as current state.
	context->_setCurrentState(new StartupState(initialState));

	auto asyncData = context->_getAsyncData();
	if(asyncData) {
		// Setup event queue.
		asyncData->eventQueue.clear();
		asyncData->hEventAvailable.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
		triggerEvent(context, event);

		// Setup worker thread in which event handling is performed.
		// entry() of initial state will be called in the thread.
		asyncData->hEventReady.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
		asyncData->hEventShutdown.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
		asyncData->hWorkerThread.Attach(CreateThread(nullptr, 0, workerThreadProc, context, 0, nullptr));
		if(!asyncData->hWorkerThread) {
			return HRESULT_FROM_WIN32(GetLastError());
		}
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
HRESULT StateMachine::shutdown(IContext* context, DWORD timeout)
{
	auto asyncData = context->_getAsyncData();
	if(asyncData && asyncData->hEventShutdown) {
		// Signal worker thread to shutdown and wait for it to terminate.
		WIN32_ASSERT_OK(SetEvent(asyncData->hEventShutdown));
		// Wait for worker thread to terminate.
		DWORD wait = WaitForSingleObject(asyncData->hWorkerThread, timeout);
		HR_ASSERT_OK(checkWaitResult(wait));
	}

	context->_setCurrentState(nullptr);

	return S_OK;
}

HRESULT StateMachine::triggerEvent(IContext * context, IEvent * event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	HR_ASSERT_OK(setupCompleted(context));

	auto asyncData = context->_getAsyncData();
	if(asyncData) {
		// Add the event to event queue and signal that event is available.
		{
			IContext::lock_t _lock(asyncData->eventQueueLock);
			asyncData->eventQueue.push_back(event);
		}
		WIN32_ASSERT_OK(SetEvent(asyncData->hEventAvailable));
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

	std::unique_ptr<IContext::lock_t> _lock(context->_getHandleEventLock());

	CComPtr<IEvent> e(event);
	IState* nextState = nullptr;
	auto currentState = context->_getCurrentState();
	HRESULT hr = forEachState(currentState, [this, context, event, &nextState](IState* state)
	{
		HRESULT hr = HR_EXPECT_OK(state->_handleEvent(context, event, &nextState));
		switch(hr) {
		case S_OK:		// Event is handled. -> Stop enumeration.
			return S_FALSE;
		case S_FALSE:	// Event is ignored. -> Master state should handle the event.
			return S_OK;
		default:
			return hr;
		}
	});
	if(hr == S_FALSE) hr = S_OK;	// forEachState() exits successfully.
	CComPtr<IState> _nextState(nextState);
	if(SUCCEEDED(hr) && nextState) {
		hr = forEachState(currentState, [this, context, event, nextState](IState* state)
		{
			return ((state != nextState) && (state != nextState->_getMasterState())) ?
				HR_EXPECT_OK(state->_exit(context, event, nextState)) :
				S_FALSE;
		});
		if(hr == S_FALSE) hr = S_OK;	// forEachState() exits successfully.
		if(SUCCEEDED(hr)) {
			CComPtr<IState> previousState(currentState);
			context->_setCurrentState(nextState);
			if(!nextState->_hasEntryCalled()) {
				nextState->_setEntryCalled(true);
				HR_ASSERT_OK(nextState->_entry(context, event, previousState));
			}
		}
	}
	return hr;
}

HRESULT StateMachine::waitReady(IContext * context, DWORD timeout)
{
	auto hr = S_OK;
	auto asyncData = context->_getAsyncData();
	if(asyncData) {
		HANDLE hEvents[] = { asyncData->hWorkerThread, asyncData->hEventReady, asyncData->hEventShutdown };
		static const DWORD eventsCount = ARRAYSIZE(hEvents);
		DWORD wait = WaitForMultipleObjects(eventsCount, hEvents, FALSE, timeout);
		HR_ASSERT_OK(checkWaitResult(wait, eventsCount));
		switch(wait) {
		case WAIT_OBJECT_0:
			// Worker thread has been terminated.
			{
				DWORD exitCode;
				hr = WIN32_EXPECT_OK(GetExitCodeThread(asyncData->hWorkerThread, &exitCode));
				if(SUCCEEDED(hr)) {
					// return exit code of worker thread.
					hr = HRESULT_FROM_WIN32(exitCode);
					// (hr == S_OK) means that worker thread has terminated(Not ready).
					if(hr == S_OK) hr = S_FALSE;
				}
			}
			break;
		case WAIT_OBJECT_0 + 1:
			// Completed to setup.
			hr = S_OK;
			break;
		case WAIT_OBJECT_0 + 2:
			// shutdown() has been called.
			hr = S_FALSE;
			break;
		default:
			// Unreachable !
			hr = E_UNEXPECTED;
			break;
		}
	}
	return hr;
}

DWORD StateMachine::workerThreadProc(LPVOID lpParameter)
{
	auto context = (IContext*)lpParameter;
	auto stateMachine = (StateMachine*)(context->_getStateMachine());
	return stateMachine->doWorkerThread(context);
}

HRESULT StateMachine::doWorkerThread(IContext* context)
{
	auto asyncData = context->_getAsyncData();
	HANDLE hEvents[] = { asyncData->hEventAvailable, asyncData->hEventShutdown };
	static const DWORD eventsCount = ARRAYSIZE(hEvents);

	auto first = true;
	do {
		// Wait for event to be queued.
		DWORD wait = WaitForMultipleObjects(eventsCount, hEvents, FALSE, INFINITE);
		if(wait < eventsCount) WIN32_ASSERT_OK(ResetEvent(hEvents[wait]));
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
				WIN32_ASSERT_OK(SetEvent(asyncData->hEventReady));
			}
		} while(true);
	} while(true);
	return S_OK;
}

HRESULT StateMachine::setupCompleted(IContext* context) const
{
	return context->_getCurrentState() ? S_OK : E_ILLEGAL_METHOD_CALL;
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

// Call function with IState object from sub state to it's master state.
// If function returns other than S_OK, enumeration stops.
HRESULT StateMachine::forEachState(IState * state, std::function<HRESULT(IState*)> func)
{
	HRESULT hr = S_OK;
	while(state) {
		hr = func(state);
		if(hr != S_OK) break;
		state = state->_getMasterState();
	}
	return hr;
}

}