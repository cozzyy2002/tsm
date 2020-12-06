#pragma once
#include <StateMachine/StateMachineMessage.h>
#include "StateMachine.h"

// Make sure that IContext is instance of AsyncContext.
#define ASSERT_ASYNC(c) HR_ASSERT(c->isAsync(), TSM_E_NOT_SUPPORTED)

namespace tsm {

class AsyncStateMachine : public StateMachine
{
public:
	AsyncStateMachine() {}
	virtual ~AsyncStateMachine() {}

	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event) override;
	virtual HRESULT shutdown(IContext* context, DWORD timeout) override;
	virtual HRESULT triggerEvent(IContext* context, IEvent* event) override;
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
	HRESULT doWorkerThread(SetupParam* param);
	HRESULT checkWaitResult(DWORD wait, DWORD eventCount = 1) const;
};

}
