#pragma once

#include <StateMachine/Interface.h>

namespace tsm {

class StateMachine : public IStateMachine
{
public:
	StateMachine();

	HRESULT setup(IContext* context, IState* initialState, IEvent* event);
	HRESULT shutdown(IContext* context);
	HRESULT triggerEvent(IContext* context, IEvent* event);
	HRESULT handleEvent(IContext* context, IEvent* event);

protected:
	HRESULT doWorkerThread(IContext* context);
	HRESULT setupCompleted(IContext* context) const;
};

}
