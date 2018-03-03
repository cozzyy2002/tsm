#include "StateMachine/stdafx.h"

#include <StateMachine/State.h>
#include <StateMachine/Assert.h>
#include "StateMachine.h"
#include "AsyncStateMachine.h"

// Make sure that IContext is instance of AsyncContext.
#define ASSERT_ASYNC(c) HR_ASSERT(c->_getAsyncData(), E_INVALIDARG)

namespace tsm {

/*static*/ IStateMachine* IStateMachine::create(IContext* context)
{
	return context->isAsync() ? new AsyncStateMachine() : new StateMachine();
}

HRESULT AsyncStateMachine::setup(IContext * context, IState * initialState, IEvent * event)
{
	// Ensure to release object on error.
	CComPtr<IState> _initialState(initialState);
	CComPtr<IEvent> _event(event);

	ASSERT_ASYNC(context);

	HR_ASSERT_OK(setupInner(context, initialState, event));

	auto asyncData = context->_getAsyncData();

	// Setup event queue.
	asyncData->eventQueue.clear();
	asyncData->hEventAvailable.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
	HR_ASSERT_OK(triggerEvent(context, event, IContext::DefaultEventPriority));

	// Setup worker thread in which event handling is performed.
	// entry() of initial state will be called in the thread.
	asyncData->hEventReady.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
	asyncData->hEventShutdown.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
	asyncData->hWorkerThread.Attach(CreateThread(nullptr, 0, workerThreadProc, context, 0, nullptr));
	if(!asyncData->hWorkerThread) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT AsyncStateMachine::shutdown(IContext * context, DWORD timeout)
{
	ASSERT_ASYNC(context);

	auto asyncData = context->_getAsyncData();
	if(asyncData->hEventShutdown) {
		// Signal worker thread to shutdown and wait for it to terminate.
		DWORD wait = SignalObjectAndWait(asyncData->hEventShutdown, asyncData->hWorkerThread, timeout, FALSE);
		HR_ASSERT_OK(checkWaitResult(wait));
	}

	HR_ASSERT_OK(shutdownInner(context, timeout));
	return S_OK;
}

HRESULT AsyncStateMachine::triggerEvent(IContext * context, IEvent * event, int priority)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	ASSERT_ASYNC(context);

	HR_ASSERT_OK(setupCompleted(context));

	auto asyncData = context->_getAsyncData();
	// Add the event to event queue and signal that event is available.
	// Events are added to the queue by priority order.
	// deque::back() returns event with highest priority.
	// Events with same priority are added by FIFO order(deque::back() returns event triggered first).
	{
		IContext::lock_t _lock(asyncData->eventQueueLock);
		auto& eventQueue = asyncData->eventQueue;
		auto it = eventQueue.begin();
		for(; it != eventQueue.end(); it++) {
			if(priority <= it->first) break;
		}
		eventQueue.insert(it, std::make_pair(priority, event));
	}
	WIN32_ASSERT(SetEvent(asyncData->hEventAvailable));
	return S_OK;
}

VOID AsyncStateMachine::timerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
}

HRESULT AsyncStateMachine::triggerDelayedEvent(ITimerClient* client, ITimerClient::Timer * pTimer, DWORD timeout, IEvent * event, int priority)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	auto hTimerQueue = client->getTimerQueue();
	if(!hTimerQueue) {
		WIN32_ASSERT(hTimerQueue = CreateTimerQueue());
		client->setTimerQueue(hTimerQueue);
	}
	client->getTimers().push_back(ITimerClient::Timer { client, NULL, event });
	CreateTimerQueueTimer(&timer.hTimer, hTimerQueue, timerCallback, , timeout, 0, 0);
	return E_NOTIMPL;
}

HRESULT AsyncStateMachine::cancelDelayedEvent(ITimerClient* client, ITimerClient::Timer * pTimer)
{
	return E_NOTIMPL;
}

HRESULT AsyncStateMachine::handleEvent(IContext * context, IEvent * event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	ASSERT_ASYNC(context);

	return StateMachine::handleEvent(context, event);
}

HRESULT AsyncStateMachine::waitReady(IContext * context, DWORD timeout)
{
	ASSERT_ASYNC(context);

	auto hr = S_OK;
	auto asyncData = context->_getAsyncData();
	HANDLE hEvents[] = { asyncData->hWorkerThread, asyncData->hEventReady, asyncData->hEventShutdown };
	static const DWORD eventsCount = ARRAYSIZE(hEvents);
	DWORD wait = WaitForMultipleObjects(eventsCount, hEvents, FALSE, timeout);
	HR_ASSERT_OK(checkWaitResult(wait, eventsCount));
	switch(wait) {
	case WAIT_OBJECT_0:
		// Worker thread has been terminated.
		{
			DWORD exitCode;
			hr = WIN32_EXPECT(GetExitCodeThread(asyncData->hWorkerThread, &exitCode));
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

	return hr;
}

DWORD AsyncStateMachine::workerThreadProc(LPVOID lpParameter)
{
	auto context = (IContext*)lpParameter;
	auto stateMachine = (AsyncStateMachine*)(context->_getStateMachine());
	auto hr = stateMachine->doWorkerThread(context);
	stateMachine->callStateMonitor(context, [&](IStateMonitor* stateMonitor)
	{
		stateMonitor->onWorkerThreadExit(context, hr);
	});
	return hr;
}

HRESULT AsyncStateMachine::doWorkerThread(IContext* context)
{
	auto asyncData = context->_getAsyncData();
	HANDLE hEvents[] = { asyncData->hEventAvailable, asyncData->hEventShutdown };
	static const DWORD eventsCount = ARRAYSIZE(hEvents);

	auto firstEvent = true;
	auto firstIdle = true;
	do {
		// Wait for event to be queued.
		DWORD wait = WaitForMultipleObjects(eventsCount, hEvents, FALSE, INFINITE);
		if(wait < eventsCount) WIN32_ASSERT(ResetEvent(hEvents[wait]));
		HR_ASSERT_OK(checkWaitResult(wait, eventsCount));
		if(wait == (WAIT_OBJECT_0 + 1)) break;
		do {
			CComPtr<IEvent> event;
			{
				// Fetch event from the queue.
				IContext::lock_t _lock(asyncData->eventQueueLock);
				auto& eventQueue = asyncData->eventQueue;
				if(eventQueue.empty()) {
					callStateMonitor(context, [&](IStateMonitor* stateMonitor)
					{
						stateMonitor->onIdle(context, firstIdle);
						firstIdle = false;
					});
					break;
				}
				event = eventQueue.back().second;
				eventQueue.pop_back();
			}

			// Handle the event.
			HR_ASSERT_OK(handleEvent(context, event));
			if(firstEvent) {
				// Set ready event after first event handled(AsyncContext::setup() is completed).
				firstEvent = false;
				WIN32_ASSERT(SetEvent(asyncData->hEventReady));
			}
		} while(true);
	} while(true);
	return S_OK;
}

// Check return value of WaitForSingleObject() and WaitForMultipleObjects().
HRESULT AsyncStateMachine::checkWaitResult(DWORD wait, DWORD eventCount /*= 1*/) const
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
