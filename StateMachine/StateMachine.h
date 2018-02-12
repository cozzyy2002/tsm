#pragma once

#include <StateMachine/Interface.h>
#include <functional>

namespace tsm {

class StateMachine : public IStateMachine
{
public:
	StateMachine() {}
	virtual ~StateMachine() {}

	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event);
	virtual HRESULT shutdown(IContext* context, DWORD timeout);
	virtual HRESULT triggerEvent(IContext* context, IEvent* event, int priority);
	virtual HRESULT handleEvent(IContext* context, IEvent* event);
	virtual HRESULT waitReady(IContext* context, DWORD timeout);

protected:
	HRESULT setupInner(IContext* context, IState* initialState, IEvent* event);
	HRESULT shutdownInner(IContext* context, DWORD timeout);
	HRESULT setupCompleted(IContext* context) const;
	HRESULT forEachState(IState* state, std::function<HRESULT(IState*)> func);
	void callStateMonitor(IContext* context, std::function<void(IStateMonitor* stateMonitor)> caller);
};

}
