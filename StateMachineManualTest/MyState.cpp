#include "stdafx.h"

#include "MyContext.h"
#include "MyEvent.h"
#include "MyState.h"

ULONG MyState::Release(void)
{
	if(1 == m_cRef) {
		m_logger->log(_T("Deleting %s"), MyObject::toString());
	}
	return Unknown::Release();
}

HRESULT MyState::handleEvent(MyContext*, MyEvent* event, MyState** nextState)
{
	m_logger->log(_T("%s::handleEvent(): HRESULT=0x%p"), MyObject::toString(), event->hrHandleEvent);

	*nextState = event->nextState;
	return event->hrHandleEvent;
}

HRESULT MyState::entry(MyContext* context, MyEvent* event, MyState* previousState)
{
	// event might be nullptr when entry() is called from Context::setup().
	auto hr = event ? event->hrEntry : S_OK;

	m_logger->log(_T("%s::entry(): HRESULT=0x%p"), MyObject::toString(), hr);
	return hr;
}

HRESULT MyState::exit(MyContext* context, MyEvent* event, MyState* nextState)
{
	auto hr = event ? event->hrExit : S_OK;

	m_logger->log(_T("%s::exit(): HRESULT=0x%p"), MyObject::toString(), hr);
	return hr;
}
