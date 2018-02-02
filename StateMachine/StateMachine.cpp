#include <StateMachine/stdafx.h>
#include <StateMachine/StateMachine.h>
#include <StateMachine/Interface.h>

namespace tsm {

StateMachine::StateMachine()
{
}

HRESULT StateMachine::setup(IContext* context, IState * initialState, IEvent* event /*= nullptr*/)
{
	context->m_currentState = initialState;
	context->m_currentState->_entry(context, event, context->m_currentState);

	return S_OK;
}

HRESULT StateMachine::shutdown(IContext* context)
{
	return E_NOTIMPL;
}

HRESULT StateMachine::handleEvent(IContext* context, IEvent * event)
{
	CComPtr<IEvent> e(event);
	IState* nextState = nullptr;
	HRESULT hr = context->m_currentState->_handleEvent(context, event, &nextState);
	CComPtr<IState> _nextState(nextState);
	if(SUCCEEDED(hr) && nextState) {
		context->m_currentState->_exit(context, event, nextState);
		CComPtr<IState> previousState(context->m_currentState);
		context->m_currentState = nextState;
		context->m_currentState->_entry(context, event, previousState);
	}
	return hr;
}

}