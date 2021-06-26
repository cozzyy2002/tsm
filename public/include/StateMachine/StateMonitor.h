#pragma once

#include "Interface.h"

namespace tsm
{

template<class C = IContext, class E = IEvent, class S = IState>
class StateMonitor : public IStateMonitor
{
public:
#pragma region Implementation of IStateMonitor
	virtual void onIdle(IContext* context) override final { onIdle((C*)context); }
	virtual void onEventTriggered(IContext* context, IEvent* event) override final { onEventTriggered((C*)context, (E*)event); }
	virtual void onEventHandling(IContext* context, IEvent* event, IState* current) override final { onEventHandling((C*)context, (E*)event, (S*)current); }

	/**
	 * When setup(), previous is nullptr.
	 * When shutdown(), next is nullptr.
	 */
	virtual void onStateChanged(IContext* context, IEvent* event, IState* previous, IState* next) override final { onStateChanged((C*)context, (E*)event, (S*)previous, (S*)next); }
	virtual void onTimerStarted(IContext* context, IEvent* event) override final { onTimerStarted((C*)context, (E*)event); }
	virtual void onTimerStopped(IContext* context, IEvent* event, HRESULT hr) override final { onTimerStopped((C*)context, (E*)event, hr); }
	virtual void onWorkerThreadExit(IContext* context, HRESULT exitCode) override final { onWorkerThreadExit((C*)context, exitCode); }
#pragma endregion

#pragma region Methods to be overridden by derived class.
	virtual void onIdle(C* context) {}
	virtual void onEventTriggered(C* context, E* event) {}
	virtual void onEventHandling(C* context, E* event, S* current) {}

	/**
	 * When setup(), previous is nullptr.
	 * When shutdown(), next is nullptr.
	 */
	virtual void onStateChanged(C* context, E* event, S* previous, S* next) {}
	virtual void onTimerStarted(C* context, E* event) {}
	virtual void onTimerStopped(C* context, E* event, HRESULT hr) {}
	virtual void onWorkerThreadExit(C* context, HRESULT exitCode) {}
#pragma endregion
};

}
