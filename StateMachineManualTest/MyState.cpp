#include "stdafx.h"

#include "MyContext.h"
#include "MyEvent.h"
#include "MyState.h"

MyState::MyState(ILogger& logger, const std::tstring& name, MyState* masterState /*= nullptr*/)
	: State(masterState), MyObject(name.c_str(), &logger), isExitCalledOnShutdown(false)
{
}

MyState::~MyState()
{
	m_logger->log(_T("Deleting %s"), MyObject::toString());
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
