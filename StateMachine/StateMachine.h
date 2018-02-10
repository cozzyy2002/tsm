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
	HRESULT triggerEvent(IContext* context, IEvent* event);
	HRESULT handleEvent(IContext* context, IEvent* event);
	HRESULT waitReady(IContext* context, DWORD timeout);

protected:
	static DWORD WINAPI workerThreadProc(LPVOID lpParameter);
	HRESULT doWorkerThread(IContext* context);
	HRESULT setupCompleted(IContext* context) const;
	HRESULT checkWaitResult(DWORD wait, DWORD eventCount = 1) const;
	HRESULT forEachState(IState* state, std::function<HRESULT(IState*)> func);
};

}
