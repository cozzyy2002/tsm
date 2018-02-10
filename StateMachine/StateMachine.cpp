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
HRESULT StateMachine::shutdown(IContext* context)
{
	auto asyncData = context->getAsyncData();
	if(asyncData && asyncData->hEventShutdown) {
		// Signal worker thread to shutdown and wait for it to terminate.
		WIN32_ASSERT_OK(SetEvent(asyncData->hEventShutdown));
		// Wait for worker thread to terminate.
		DWORD wait = WaitForSingleObject(asyncData->hWorkerThread, 100);
		HR_ASSERT_OK(checkWaitResult(wait));
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
	auto hr = S_OK;
	auto asyncData = context->getAsyncData();
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
	auto stateMachine = (StateMachine*)(context->m_stateMachine.get());
	return stateMachine->doWorkerThread(context);
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