#include <StateMachine/stdafx.h>
#include <StateMachine/Interface.h>
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

HRESULT StateMachine::setup(IContext* context, IState * initialState, IEvent* event)
{
	context->m_currentState = initialState;
	HR_ASSERT_OK(context->m_currentState->_entry(context, event, nullptr));

	return S_OK;
}

HRESULT StateMachine::shutdown(IContext* context)
{
	return E_NOTIMPL;
}

HRESULT StateMachine::triggerEvent(IContext * context, IEvent * event)
{
	auto asyncData = context->getAsyncData();
	if(asyncData) {
		return E_NOTIMPL;
	} else {
		// Async operation is not supported.
		return E_ILLEGAL_METHOD_CALL;
	}
}

HRESULT StateMachine::handleEvent(IContext* context, IEvent * event)
{
	if(!context->m_currentState) {
		// Current state has not been set.
		return E_ILLEGAL_METHOD_CALL;
	}

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

}