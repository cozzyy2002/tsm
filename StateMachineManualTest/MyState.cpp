#include "stdafx.h"

#include "MyContext.h"
#include "MyEvent.h"
#include "MyState.h"

HRESULT MyState::handleEvent(MyContext*, MyEvent* event, MyState** nextState)
{
	*nextState = event->nextState;
	return event->hrHandleEvent;
}

HRESULT MyState::entry(MyContext* context, MyEvent* event, MyState* previousState)
{
	// event might be nullptr when entry() is called from Context::setup().
	return event ? event->hrEntry : S_OK;
}

HRESULT MyState::exit(MyContext* context, MyEvent* event, MyState* nextState)
{
	return event->hrExit;
}
