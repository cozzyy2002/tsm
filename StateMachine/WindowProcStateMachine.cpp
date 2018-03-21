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

	auto asyncData = context->_getHandle()->asyncData;
	HR_ASSERT_OK(asyncData->queueEvent(event));

	HR_ASSERT_OK(postMessage(context, MessageType::HandleEvent));
	return S_OK;
}

HRESULT WindowProcStateMachine::waitReady(IContext* context, DWORD timeout)
{
	return E_NOTIMPL;
}

HRESULT WindowProcStateMachine::postMessage(IContext* context, MessageType type, Message* message /*= nullptr*/)
{
	if(!message) message = new Message();
	message->_this = this;
	message->context = context;
	auto asyncData = context->_getHandle()->asyncData;
	auto hr = WIN32_EXPECT(PostMessage(asyncData->hWnd, asyncData->msg, (WPARAM)type, (LPARAM)message));
	if(FAILED(hr)) {
		delete message;
	}
	return hr;
}

LRESULT IStateMachine::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto type = (WindowProcStateMachine::MessageType)wParam;
	auto message = (WindowProcStateMachine::Message*)lParam;
	HR_EXPECT_OK(message->_this->windowProc(type, message));

	return 0L;
}

HRESULT WindowProcStateMachine::windowProc(MessageType type, Message* message)
{
	// Message object and it's contents will be deleted when this method exits.
	std::unique_ptr<Message> _message(message);

	auto context = message->context;
	switch(type) {
	case MessageType::Setup:
		setupProc(context, message->event);
		break;
	case MessageType::Shutdown:
		break;
	case MessageType::HandleEvent:
		handleEventProc(context);
		break;
	}
	return S_OK;
}

HRESULT WindowProcStateMachine::setupProc(IContext* context, IEvent* event)
{
	auto asyncData = context->_getHandle()->asyncData;
	// Call entry() of initial state.
	HR_ASSERT_OK(callEntry(context->_getCurrentState(), context, event, nullptr));
	// Notify that setup completed.
	WIN32_ASSERT(SetEvent(asyncData->hEventReady));
	return S_OK;
}

HRESULT WindowProcStateMachine::handleEventProc(IContext* context)
{
	CComPtr<IEvent> event;
	size_t eventCountLeft = 0;
	{
		// Fetch event from the queue.
		auto asyncData = context->_getHandle()->asyncData;
		lock_t _lock(asyncData->eventQueueLock);
		auto& eventQueue = asyncData->eventQueue;
		HR_ASSERT(!eventQueue.empty(), E_UNEXPECTED);
		event = eventQueue.back();
		eventQueue.pop_back();
		eventCountLeft = eventQueue.size();
	}

	// Handle the event.
	HR_ASSERT_OK(handleEvent(context, event));
	if(eventCountLeft) {
		// Post message to handle next event in the queue.
		// Do not loop to handle all events in the queue so that app can process own messages.
		HR_ASSERT_OK(postMessage(context, MessageType::HandleEvent));
	} else {
		context->_getHandle()->callStateMonitor(context, [](IContext* context, IStateMonitor* stateMonitor)
		{
			stateMonitor->onIdle(context);
		});
	}

	return S_OK;
}

}
