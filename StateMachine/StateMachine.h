#pragma once

#include <StateMachine/Interface.h>
#include <functional>

namespace tsm {

class StateMachine : public IStateMachine
{
public:
	StateMachine();

	HRESULT setup(IContext* context, IState* initialState, IEvent* event);
	HRESULT shutdown(IContext* context, DWORD timeout);
	HRESULT triggerEvent(IContext* context, IEvent* event) { return E_NOTIMPL; }
	HRESULT handleEvent(IContext* context, IEvent* event);
	HRESULT waitReady(IContext* context, DWORD timeout);

protected:
	HRESULT setupInner(IContext* context, IState* initialState, IEvent* event);
	HRESULT shutdownInner(IContext* context, DWORD timeout);
	HRESULT setupCompleted(IContext* context) const;
	HRESULT forEachState(IState* state, std::function<HRESULT(IState*)> func);
};

}
