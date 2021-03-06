#pragma once

#include <StateMachine/Interface.h>
#include <functional>

namespace tsm {

class StateMachine : public IStateMachine
{
public:
	StateMachine() {}
	virtual ~StateMachine() {}

	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event) override;
	virtual HRESULT shutdown(IContext* context, DWORD timeout) override;
	virtual HRESULT triggerEvent(IContext* context, IEvent* event) override;
	virtual HRESULT handleEvent(IContext* context, IEvent* event) override;
	virtual HRESULT waitReady(IContext* context, DWORD timeout) override;

protected:
	HRESULT setupInner(IContext* context, IState* initialState, IEvent* event);
	HRESULT shutdownInner(IContext* context, DWORD timeout);
	bool isSetupCompleted(IContext* context) const;
	HRESULT forEachState(IState* state, std::function<HRESULT(IState*)> func);

	HRESULT callHandleEvent(IState* state, IContext* context, IEvent* event, IState** nextState);
	HRESULT callEntry(IState* state, IContext* context, IEvent* event, IState* previousState);
	HRESULT callExit(IState* state, IContext* context, IEvent* event, IState* nextState);
};

}
