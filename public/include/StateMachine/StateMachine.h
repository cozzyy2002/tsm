#pragma once

#include <winerror.h>

namespace tsm {

class IContext;
class IState;
class IEvent;

class StateMachine
{
public:
	StateMachine();

	HRESULT setup(IContext* context, IState* initialState, IEvent* event = nullptr);
	HRESULT shutdown(IContext* context);

	HRESULT triggerEvent(IContext* context, IEvent* event);
	HRESULT handleEvent(IContext* context, IEvent* event);
};

}
