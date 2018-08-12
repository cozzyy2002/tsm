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
	m_logger->log(_T("%s::handleEvent()"), MyObject::toString());

	*nextState = event->nextState;
	return event->hrHandleEvent;
}

HRESULT MyState::entry(MyContext* context, MyEvent* event, MyState* previousState)
{
	m_logger->log(_T("%s::entry()"), MyObject::toString());

	// event might be nullptr when entry() is called from Context::setup().
	return event ? event->hrEntry : S_OK;
}

HRESULT MyState::exit(MyContext* context, MyEvent* event, MyState* nextState)
{
	m_logger->log(_T("%s::exit()"), MyObject::toString());

	return event ? event->hrExit : S_OK;
}
