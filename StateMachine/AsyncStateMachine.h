#pragma once
#include "StateMachine.h"

namespace tsm {

class AsyncStateMachine : public StateMachine
{
public:
	AsyncStateMachine() {}
	virtual ~AsyncStateMachine() {}

	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event);
	virtual HRESULT shutdown(IContext* context, DWORD timeout);
	virtual HRESULT triggerEvent(IContext* context, IEvent* event);
	virtual HRESULT handleEvent(IContext* context, IEvent* event);
	virtual HRESULT waitReady(IContext* context, DWORD timeout);

protected:
	static DWORD WINAPI workerThreadProc(LPVOID lpParameter);
	HRESULT doWorkerThread(IContext* context);
	HRESULT checkWaitResult(DWORD wait, DWORD eventCount = 1) const;
};

}
