#include "StateMachine/stdafx.h"

#include <StateMachine/State.h>
#include <StateMachine/Assert.h>
#include "StateMachine.h"
#include "AsyncStateMachine.h"
#include "Handles.h"

namespace tsm {

/**
 * Returns StateMachine object for Context and AsyncStateMachine object for AsyncContext respectively.
 */
/*static*/ IStateMachine* IStateMachine::create(IContext* context)
{
	return context->isAsync() ? new AsyncStateMachine() : new StateMachine();
}

class DefaultAsyncDispatcher : public IAsyncDispatcher
{
public:
	virtual HANDLE dispatch(Method method, LPVOID param) override
	{
		return CreateThread(nullptr, 0, method, param, 0, nullptr);
	}
};

HRESULT AsyncStateMachine::setup(IContext * context, IState * initialState, IEvent * event)
{
	// Ensure to release object on error.
	CComPtr<IState> _initialState(initialState);
	CComPtr<IEvent> _event(event);

	ASSERT_ASYNC(context);

	HR_ASSERT_OK(setupInner(context, initialState, event));

	auto asyncData = context->_getHandle(true)->asyncData;

	// Setup event queue.
	asyncData->eventQueue.clear();
	asyncData->hEventAvailable.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));

	// Setup worker thread in which event handling is performed.
	// entry() of initial state will be called in the thread.
	asyncData->hEventReady.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
	asyncData->hEventShutdown.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
	// param will be deleted in worker thread.
	auto param = new SetupParam(context, this, event);

	auto dispatcher = context->_createAsyncDispatcher();
	if(!dispatcher) { dispatcher = new DefaultAsyncDispatcher(); }
	asyncData->asyncDispatcher.reset(dispatcher);
	asyncData->hWorkerThread.Attach(dispatcher->dispatch(workerThreadProc, param));
	if(!asyncData->hWorkerThread) {
		delete param;
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT AsyncStateMachine::shutdown(IContext * context, DWORD timeout)
{
	ASSERT_ASYNC(context);

	auto asyncData = context->_getHandle()->asyncData;
	if(asyncData->hEventShutdown) {
		// Signal worker thread to shutdown and wait for it to terminate.
		DWORD wait = SignalObjectAndWait(asyncData->hEventShutdown, asyncData->hWorkerThread, timeout, FALSE);
		HR_ASSERT_OK(checkWaitResult(wait));
	}

	HR_ASSERT_OK(shutdownInner(context, timeout));
	return S_OK;
}

HRESULT AsyncStateMachine::triggerEvent(IContext* context, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	ASSERT_ASYNC(context);
	HR_ASSERT_OK(setupCompleted(context));
	HR_ASSERT(event, E_INVALIDARG);

	auto timerClient = event->_getTimerClient();
	if(timerClient && !event->_getHandle()->isTimerCreated) {
		// Event should be handled after delay time elapsed.
		return HR_EXPECT_OK(timerClient->_setEventTimer(TimerClient::TimerType::TriggerEvent, context, event));
	}

	auto asyncData = context->_getHandle()->asyncData;

	// Add the event to event queue and signal that event is available.
	HR_ASSERT_OK(asyncData->queueEvent(event));
	WIN32_ASSERT(SetEvent(asyncData->hEventAvailable));

	context->_getHandle()->callStateMonitor(context, [event](IContext* context, IStateMonitor* stateMonitor)
	{
		stateMonitor->onEventTriggered(context, event);
	});
	return S_OK;
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
	auto asyncData = context->_getHandle()->asyncData;
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
	// Call worker thread procedure.
	// Note: param should been deleted by doWorkerThead() method.
	auto param = (SetupParam*)lpParameter;
	auto context = param->context;
	auto hr = param->stateMachine->doWorkerThread(param);

	// Notify that worker thread exits.
	context->_getHandle()->callStateMonitor(context, [hr](IContext* context, IStateMonitor* stateMonitor)
	{
		stateMonitor->onWorkerThreadExit(context, hr);
	});
	return hr;
}

HRESULT AsyncStateMachine::doWorkerThread(SetupParam* param)
{
	auto context = param->context;
	auto asyncData = context->_getHandle()->asyncData;
	{
		// SetupParam object and objects in SetupParam will be deleted when out of this scope.
		std::unique_ptr<SetupParam> _param(param);
		// Call entry() of initial state.
		HR_ASSERT_OK(callEntry(context->_getCurrentState(), context, param->event, nullptr));

		// Call StateMonitor::onStateChanged() with nullptr as previous state.
		context->_getHandle()->callStateMonitor(context, [&](IContext* context, IStateMonitor* stateMonitor)
		{
			stateMonitor->onStateChanged(context, param->event, nullptr, context->_getCurrentState());
		});
		// Notify that setup completed.
		WIN32_ASSERT(SetEvent(asyncData->hEventReady));
	}

	HANDLE hEvents[] = { asyncData->hEventAvailable, asyncData->hEventShutdown };
	static const DWORD eventsCount = ARRAYSIZE(hEvents);

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
				lock_t _lock(asyncData->eventQueueLock);
				auto& eventQueue = asyncData->eventQueue;
				if(eventQueue.empty()) {
					context->_getHandle()->callStateMonitor(context, [](IContext* context, IStateMonitor* stateMonitor)
					{
						stateMonitor->onIdle(context);
					});
					break;
				}
				event = eventQueue.back();
				eventQueue.pop_back();
			}

			// Handle the event.
			HR_ASSERT_OK(handleEvent(context, event));
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
