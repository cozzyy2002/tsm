#pragma once
#include "AsyncStateMachine.h"

namespace tsm {

class WindowProcStateMachine : public AsyncStateMachine
{
public:
	WindowProcStateMachine(HWND hWnd, UINT msg) : m_hWnd(hWnd), m_msg(msg) {}
	virtual ~WindowProcStateMachine() {}

	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event) override;

	virtual HRESULT shutdown(IContext* context, DWORD timeout) override;
	virtual HRESULT triggerEvent(IContext* context, IEvent* event) override;
	// handleEvent() method is same as AsyncStateMachine class.
	//virtual HRESULT handleEvent(IContext* context, IEvent* event) override;
	virtual HRESULT waitReady(IContext* context, DWORD timeout) override;

protected:
	// Message type to be passed to the window procedure as WPARAM
	enum class MessageType {
		Setup,
		Shutdown,
		HandleEvent,
	};

	// Message structure to be passed to the window procedure as LPARAM
	struct Message {
		WindowProcStateMachine* _this;
		IContext* context;				// type = Setup, Shutdown, HandleEvent
		CComPtr<IEvent> event;			// type = Setup

		Message();
	};

	HRESULT postMessage(IContext* context, MessageType type, Message* message = nullptr);
	HRESULT windowProc(MessageType type, Message* message);
	HRESULT setupProc(IContext* context, IEvent* event);
	HRESULT handleEventProc(IContext* context);

	// Window procedure of WindowProcStateMachine that is called by app
	// when app window proc receives state machine message.
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma region Parameters passed to constructor.
	HWND m_hWnd;
	UINT m_msg;
#pragma endregion
};

}
