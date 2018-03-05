#pragma once
#include "StateMachine.h"

namespace tsm {

class AsyncStateMachine : public StateMachine
{
public:
	AsyncStateMachine() {}
	virtual ~AsyncStateMachine() {}

	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event) override;
	virtual HRESULT shutdown(IContext* context, DWORD timeout) override;
	virtual HRESULT triggerEvent(IContext* context, IEvent* event) override;
	virtual HRESULT triggerDelayedEvent(ITimerClient* client, ITimerClient::Timer* pTimer, DWORD timeout, IEvent* event) override;
	virtual HRESULT cancelDelayedEvent(ITimerClient* client, ITimerClient::Timer* pTimer) override;
	virtual HRESULT handleEvent(IContext* context, IEvent* event) override;
	virtual HRESULT waitReady(IContext* context, DWORD timeout) override;

protected:
	class SetupParam {
	public:
		SetupParam(IContext* context, AsyncStateMachine* stateMachine, IEvent* event)
			: context(context), stateMachine(stateMachine), event(event) {}

		IContext* context;
		AsyncStateMachine* stateMachine;
		CComPtr<IEvent> event;
	};

	static DWORD WINAPI workerThreadProc(LPVOID lpParameter);
	static VOID CALLBACK timerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired);
	HRESULT doWorkerThread(SetupParam* param);
	HRESULT checkWaitResult(DWORD wait, DWORD eventCount = 1) const;
	HANDLE getTimerQueue(ITimerClient* client);
};

}
