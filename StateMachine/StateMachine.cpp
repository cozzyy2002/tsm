#include <StateMachine/stdafx.h>

#include <StateMachine/State.h>
#include <StateMachine/TimerClient.h>
#include <StateMachine/Assert.h>
#include "StateMachine.h"
#include "Handles.h"

/*static*/ tsm::IContext::OnAssertFailed *tsm::IContext::onAssertFailedProc = nullptr;

HRESULT checkHResult(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	if(FAILED(hr)) {
		if(tsm::IContext::onAssertFailedProc) {
			tsm::IContext::onAssertFailedProc(hr, exp, sourceFile, line);
		} else {
			_tprintf_s(_T("'%s' failed: HRESULT=0x%08x at:\n%s:%d\n"), exp, hr, sourceFile, line);
		}
	}
	return hr;
}

namespace tsm {

HRESULT StateMachine::setupInner(IContext* context, IState * initialState, IEvent* /*event*/)
{
	// Check if setup() has not been called.
	HR_ASSERT(FAILED(setupCompleted(context)), E_ILLEGAL_METHOD_CALL);

	// Set StartupState object as current state.
	context->_setCurrentState(initialState);

	return S_OK;
}

HRESULT StateMachine::setup(IContext* context, IState * initialState, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IState> _initialState(initialState);
	CComPtr<IEvent> _event(event);

	HR_ASSERT_OK(setupInner(context, initialState, event));

	// Call entry() of initial state in the caller thread.
	// Pass nullptr as previous state to intial state.
	HR_ASSERT_OK(callEntry(initialState, context, event, nullptr));

	// Call StateMonitor::onStateChanged() with nullptr as previous state.
	context->_getHandle()->callStateMonitor(context, [&](IContext* context, IStateMonitor* stateMonitor)
	{
		stateMonitor->onStateChanged(context, event, nullptr, context->_getCurrentState());
	});

	return S_OK;
}

HRESULT StateMachine::shutdownInner(IContext* context, DWORD timeout)
{
	// Cleanup existing states and their pending event timers.
	forEachState(context->_getCurrentState(), [this, context](IState* state)
	{
		return HR_EXPECT_OK(callExit(state, context, nullptr, nullptr));
	});

	// Cancel pending event timers of context.
	HR_EXPECT_OK(context->_getTimerClient()->cancelAllEventTimers());

	context->_setCurrentState(nullptr);
	return S_OK;
}

/*
 * Shutdown state machine.
 *
 * This method can be called even if setup() has been called.
 */
HRESULT StateMachine::shutdown(IContext* context, DWORD timeout)
{
	HR_ASSERT_OK(shutdownInner(context, timeout));
	return S_OK;
}

HRESULT StateMachine::triggerEvent(IContext * context, IEvent * event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	return E_NOTIMPL;
}

HRESULT StateMachine::handleEvent(IContext* context, IEvent * event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	HR_ASSERT_OK(setupCompleted(context));
	HR_ASSERT(event, E_INVALIDARG);

	auto timerClient = event->_getTimerClient();
	if(timerClient && !event->_getHandle()->isTimerCreated) {
		// Event should be handled after delay time elapsed.
		return HR_EXPECT_OK(timerClient->_setEventTimer(TimerClient::TimerType::HandleEvent, context, event));
	}

	std::unique_ptr<lock_t> _lock(context->_getHandle()->getHandleEventLock());

	IState* nextState = nullptr;
	auto currentState = context->_getCurrentState();

	context->_getHandle()->callStateMonitor(context, [event, currentState](IContext* context, IStateMonitor* stateMonitor)
	{
		stateMonitor->onEventHandling(context, event, currentState);
	});

	HRESULT hr = event->_preHandle(context);
	switch(hr) {
	case S_OK:
		break;
	case S_FALSE:		// No more handling is required.
	default:			// Error occurred.
		return hr;
	}

	hr = forEachState(currentState, [this, context, event, &nextState](IState* state)
	{
		HRESULT hr = HR_EXPECT_OK(callHandleEvent(state, context, event, &nextState));
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
				HR_EXPECT_OK(callExit(state, context, event, nextState)) :
				S_FALSE;
		});
		if(hr == S_FALSE) hr = S_OK;	// forEachState() exits successfully.
		if(SUCCEEDED(hr)) {
			CComPtr<IState> previousState(currentState);
			context->_setCurrentState(nextState);
			HR_ASSERT_OK(callEntry(nextState, context, event, previousState));
			context->_getHandle()->callStateMonitor(context, [&](IContext* context, IStateMonitor* stateMonitor)
			{
				stateMonitor->onStateChanged(context, _event, previousState, nextState);
			});
		}
	}

	hr = event->_postHandle(context, hr);

	return hr;
}

HRESULT StateMachine::waitReady(IContext * context, DWORD timeout)
{
	// Nothing to do.

	return S_OK;
}

HRESULT StateMachine::setupCompleted(IContext* context) const
{
	return context->_getCurrentState() ? S_OK : E_ILLEGAL_METHOD_CALL;
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

HRESULT StateMachine::callHandleEvent(IState* state, IContext* context, IEvent* event, IState** nextState)
{
	return state->_handleEvent(context, event, nextState);
}

HRESULT StateMachine::callEntry(IState* state, IContext* context, IEvent* event, IState* previousState)
{
	auto sh = state->_getHandle();
	if(!sh->entryCalled) {
		sh->entryCalled = true;
		return state->_entry(context, event, previousState);
	} else {
		return S_OK;
	}
}

HRESULT StateMachine::callExit(IState* state, IContext* context, IEvent* event, IState* nextState)
{
	state->_getHandle()->entryCalled = false;
	HR_ASSERT_OK(state->_getTimerClient()->cancelAllEventTimers());

	// On shutdown, nextState is nullptr.
	return ((nextState != nullptr) || state->_callExitOnShutdown()) ?
		state->_exit(context, event, nextState) :
		S_OK;
}

}