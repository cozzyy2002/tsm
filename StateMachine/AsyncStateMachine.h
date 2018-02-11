#pragma once
#include "StateMachine.h"

namespace tsm {

class AsyncStateMachine : public StateMachine
{
public:
	AsyncStateMachine();
	~AsyncStateMachine();

	HRESULT setup(IContext* context, IState* initialState, IEvent* event);
	HRESULT shutdown(IContext* context, DWORD timeout);
	HRESULT triggerEvent(IContext* context, IEvent* event);
	HRESULT handleEvent(IContext* context, IEvent* event);
	HRESULT waitReady(IContext* context, DWORD timeout);

protected:
	static DWORD WINAPI workerThreadProc(LPVOID lpParameter);
	HRESULT doWorkerThread(IContext* context);
	HRESULT checkWaitResult(DWORD wait, DWORD eventCount = 1) const;
};

}
