#include "stdafx.h"

#include "MyContext.h"
#include "MyEvent.h"
#include "MyState.h"

HRESULT InitialState::handleEvent(MyContext*, MyEvent* event, MyState** nextState)
{
	*nextState = new StateA();
	return S_OK;
}

HRESULT InitialState::entry(MyContext* context, MyEvent* event, MyState* previousState)
{
	return context->triggerEvent(new MyEvent());
}
