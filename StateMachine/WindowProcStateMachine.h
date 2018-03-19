#pragma once
#include "AsyncStateMachine.h"

namespace tsm {

class WindowProcStateMachine : public AsyncStateMachine
{
public:
	WindowProcStateMachine() {}
	virtual ~WindowProcStateMachine() {}

	// This method is not implemented.
	// Use non virtual setup() method that accept HWND and UINT parameters.
	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event) override;
	HRESULT setup(HWND hWnd, UINT msg, IContext* context, IState* initialState, IEvent* event);

	virtual HRESULT shutdown(IContext* context, DWORD timeout) override;
	virtual HRESULT triggerEvent(IContext* context, IEvent* event) override;
	// handleEvent() method is same as AsyncStateMachine class.
	//virtual HRESULT handleEvent(IContext* context, IEvent* event) override;
	virtual HRESULT waitReady(IContext* context, DWORD timeout) override;

	// Window procedure that is called by app
	// when app window proc receives state machine message.
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	enum class MessageType {
		Setup,
		Shutdown,
		TriggerEvent,
		HandleEvent,
	};

	struct Message {
		WindowProcStateMachine* _this;
		IContext* context;				// type = Setup, Shutdown, TriggerEvent, HandleEvent
		CComPtr<IState> initialState;	// type = Setup
		CComPtr<IEvent> event;			// type = Setup, TriggerEvent, HandleEvent
	};

	HRESULT postMessage(IContext* context, MessageType type, Message* message);
	HRESULT windowProc(MessageType type, Message* message);
};

}
