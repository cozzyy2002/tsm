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
	virtual HRESULT triggerDelayedEvent(ITimerClient* client, ITimerClient::Timer* pTimer, DWORD timeout, IEvent* event) override;
	virtual HRESULT cancelDelayedEvent(ITimerClient* client, ITimerClient::Timer* pTimer) override;
	virtual HRESULT handleEvent(IContext* context, IEvent* event) override;
	virtual HRESULT waitReady(IContext* context, DWORD timeout) override;

protected:
	HRESULT setupInner(IContext* context, IState* initialState, IEvent* event);
	HRESULT shutdownInner(IContext* context, DWORD timeout);
	HRESULT setupCompleted(IContext* context) const;
	HRESULT forEachState(IState* state, std::function<HRESULT(IState*)> func);
	void callStateMonitor(IContext* context, std::function<void(IContext* context, IStateMonitor* stateMonitor)> caller);
};

}
