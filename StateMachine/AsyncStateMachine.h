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
	virtual HRESULT triggerEvent(IContext* context, IEvent* event, int priority) override;
	virtual HRESULT triggerDelayedEvent(ITimerClient* client, ITimerClient::Timer* pTimer, DWORD timeout, IEvent* event, int priority) override;
	virtual HRESULT cancelDelayedEvent(ITimerClient* client, ITimerClient::Timer* pTimer) override;
	virtual HRESULT handleEvent(IContext* context, IEvent* event) override;
	virtual HRESULT waitReady(IContext* context, DWORD timeout) override;

protected:
	static DWORD WINAPI workerThreadProc(LPVOID lpParameter);
	static VOID CALLBACK timerCallback(_In_ PVOID   lpParameter, _In_ BOOLEAN TimerOrWaitFired);
	HRESULT doWorkerThread(IContext* context);
	HRESULT checkWaitResult(DWORD wait, DWORD eventCount = 1) const;
	HANDLE getTimerQueue(ITimerClient* client);
};

}
