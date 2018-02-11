#include <StateMachine/stdafx.h>

#include <StateMachine/State.h>
#include <StateMachine/Assert.h>
#include "StateMachine.h"

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

HRESULT StateMachine::setupInner(IContext* context, IState * initialState, IEvent* event)
{
	// Check if setup() has not been called.
	HR_ASSERT(FAILED(setupCompleted(context)), E_ILLEGAL_METHOD_CALL);

	// Set StartupState object as current state.
	context->_setCurrentState(new StartupState(initialState));

	return S_OK;
}

HRESULT StateMachine::setup(IContext* context, IState * initialState, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IState> _initialState(initialState);
	CComPtr<IEvent> _event(event);

	HR_ASSERT_OK(setupInner(context, initialState, event));

	// Call entry() of initial state in the caller thread.
	HR_ASSERT_OK(handleEvent(context, event));

	return S_OK;
}

HRESULT StateMachine::shutdownInner(IContext* context, DWORD timeout)
{
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

HRESULT StateMachine::triggerEvent(IContext * context, IEvent * event, int priority)
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

	std::unique_ptr<IContext::lock_t> _lock(context->_getHandleEventLock());

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

}