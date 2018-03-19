#include <StateMachine/stdafx.h>

#include <StateMachine/State.h>
#include <StateMachine/Assert.h>
#include "WindowProcStateMachine.h"
#include "Handles.h"

namespace tsm {

HRESULT WindowProcStateMachine::setup(IContext* context, IState* initialState, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IState> _initialState(initialState);
	CComPtr<IEvent> _event(event);

	return E_NOTIMPL;
}

HRESULT WindowProcStateMachine::setup(HWND hWnd, UINT msg, IContext* context, IState* initialState, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IState> _initialState(initialState);
	CComPtr<IEvent> _event(event);

	ASSERT_ASYNC(context);

	HR_ASSERT_OK(setupInner(context, initialState, event));

	auto asyncData = context->_getHandle()->asyncData;
	asyncData->hWnd = hWnd;
	asyncData->msg = msg;

	auto message = new Message();
	message->initialState = initialState;
	message->event = event;
	HR_ASSERT_OK(postMessage(context, MessageType::Setup, message));
	return S_OK;
}

HRESULT WindowProcStateMachine::shutdown(IContext* context, DWORD timeout)
{
	return E_NOTIMPL;
}

HRESULT WindowProcStateMachine::triggerEvent(IContext* context, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	ASSERT_ASYNC(context);

	auto message = new Message();
	message->event = event;
	HR_ASSERT_OK(postMessage(context, MessageType::TriggerEvent, message));
	return S_OK;
}

HRESULT WindowProcStateMachine::waitReady(IContext* context, DWORD timeout)
{
	return E_NOTIMPL;
}

HRESULT WindowProcStateMachine::postMessage(IContext* context, MessageType type, Message* message)
{
	auto asyncData = context->_getHandle()->asyncData;
	message->context = context;
	WIN32_ASSERT(PostMessage(asyncData->hWnd, asyncData->msg, (WPARAM)type, (LPARAM)message));
	return S_OK;
}

LRESULT WindowProcStateMachine::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto type = (MessageType)wParam;
	auto message = (Message*)lParam;
	HR_EXPECT_OK(message->_this->windowProc(type, message));

	return 0L;
}

HRESULT WindowProcStateMachine::windowProc(MessageType type, Message * message)
{
	std::unique_ptr<Message> _message(message);

	auto context = message->context;
	switch(type) {
	case MessageType::Setup:
		HR_ASSERT_OK(message->initialState->_entry(context, message->event, nullptr));
		break;
	case MessageType::Shutdown:
		break;
	case MessageType::TriggerEvent:
		break;
	case MessageType::HandleEvent:
		HR_ASSERT_OK(handleEvent(context, message->event));
		break;
	}
	return S_OK;
}

}
