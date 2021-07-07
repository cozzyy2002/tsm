#include <StateMachine/stdafx.h>

#include <StateMachine/State.h>
#include <StateMachine/Assert.h>
#include "WindowProcStateMachine.h"
#include "Handles.h"

namespace tsm {

static const TCHAR windowPropertyName[] = _T("WindowProcStateMachine");

/*static*/ IStateMachine* IStateMachine::create(HWND hWnd, UINT msg)
{
	return new WindowProcStateMachine(hWnd, msg);
}

HRESULT WindowProcStateMachine::setup(IContext* context, IState* initialState, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IState> _initialState(initialState);
	CComPtr<IEvent> _event(event);

	ASSERT_ASYNC(context);

	HR_ASSERT_OK(setupInner(context, initialState, event));

	auto asyncData = context->_getHandle(true)->asyncData;
	asyncData->hWnd = m_hWnd;
	asyncData->msg = m_msg;

	// Subclass the hWnd. 
	WIN32_ASSERT(asyncData->appWndProc = (WNDPROC)SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc));
	WIN32_ASSERT(SetProp(m_hWnd, windowPropertyName, (HANDLE)context));

	asyncData->hEventReady.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
	asyncData->hEventShutdown.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
	auto message = new Message();
	message->event = event;
	HR_ASSERT_OK(postMessage(context, MessageType::Setup, message));
	return S_OK;
}

HRESULT WindowProcStateMachine::shutdown(IContext* context, DWORD timeout)
{
	ASSERT_ASYNC(context);

	auto asyncData = context->_getHandle()->asyncData;

	if(asyncData->hEventShutdown) {
		// Signal shutdown.
		WIN32_EXPECT(SetEvent(asyncData->hEventShutdown));
	}

	// Unsubclass the hWnd. 
	if(asyncData->hWnd) {
		WIN32_EXPECT(WndProc == (WNDPROC)SetWindowLongPtr(asyncData->hWnd, GWLP_WNDPROC, (LONG_PTR)asyncData->appWndProc));
		WIN32_EXPECT(context == (IContext*)RemoveProp(asyncData->hWnd, windowPropertyName));
		asyncData->hWnd = NULL;
	}

	HR_ASSERT_OK(shutdownInner(context, timeout));
	return S_OK;
}

HRESULT WindowProcStateMachine::triggerEvent(IContext* context, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	ASSERT_ASYNC(context);
	HR_ASSERT(isSetupCompleted(context), TSM_E_SETUP_HAS_NOT_BEEN_MADE);
	HR_ASSERT(event, E_INVALIDARG);

	auto timerClient = event->_getTimerClient();
	if(timerClient && !event->_getHandle()->isTimerCreated) {
		// Event should be handled after delay time elapsed.
		return HR_EXPECT_OK(timerClient->_setEventTimer(TimerClient::TimerType::TriggerEvent, context, event));
	}

	auto asyncData = context->_getHandle()->asyncData;

	// Add the event to event queue and post message to handle event in the queue.
	HR_ASSERT_OK(asyncData->queueEvent(event));
	HR_ASSERT_OK(postMessage(context, MessageType::HandleEvent));

	context->_getHandle()->callStateMonitor(context, [event](IContext* context, IStateMonitor* stateMonitor)
	{
		stateMonitor->onEventTriggered(context, event);
	});
	return S_OK;
}

HRESULT WindowProcStateMachine::waitReady(IContext* context, DWORD timeout)
{
	ASSERT_ASYNC(context);

	auto hr = S_OK;
	auto asyncData = context->_getHandle()->asyncData;
	HANDLE hEvents[] = { asyncData->hEventReady, asyncData->hEventShutdown };
	static const DWORD eventsCount = ARRAYSIZE(hEvents);
	DWORD wait = WaitForMultipleObjects(eventsCount, hEvents, FALSE, timeout);
	HR_ASSERT_OK(checkWaitResult(wait, eventsCount));
	switch(wait) {
	case WAIT_OBJECT_0:
		// Completed to setup.
		hr = S_OK;
		break;
	case WAIT_OBJECT_0 + 1:
		// shutdown() has been called.
		hr = S_FALSE;
		break;
	default:
		// Unreachable !
		hr = E_UNEXPECTED;
		break;
	}

	return hr;
}

WindowProcStateMachine::Message::Message()
{
	_this = nullptr;
	context = nullptr;
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

LRESULT WindowProcStateMachine::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto context = (IContext*)GetProp(hWnd, windowPropertyName);
	if(context) {
		auto asyncData = context->_getHandle()->asyncData;
		if((asyncData->msg == msg) && lParam) {
			auto type = (MessageType)wParam;
			auto message = (Message*)lParam;
			HR_EXPECT_OK(message->_this->windowProc(type, message));
		} else {
			// Call app window procedure.
			return CallWindowProc(asyncData->appWndProc, hWnd, msg, wParam, lParam);
		}
	} else {
		HR_ERROR(_T(__FUNCTION__) _T(" called with unknown HWND"), E_UNEXPECTED);
	}

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

	// Call StateMonitor::onStateChanged() with nullptr as previous state.
	context->_getHandle()->callStateMonitor(context, [&](IContext* context, IStateMonitor* stateMonitor)
	{
		stateMonitor->onStateChanged(context, event, nullptr, context->_getCurrentState());
	});
	// Notify that setup completed.
	WIN32_ASSERT(SetEvent(asyncData->hEventReady));
	return S_OK;
}

HRESULT WindowProcStateMachine::handleEventProc(IContext* context)
{
	auto asyncData = context->_getHandle()->asyncData;
	auto& eventQueue = asyncData->eventQueue;
	CComPtr<IEvent> event;
	{
		// Fetch event from the queue.
		lock_t _lock(asyncData->eventQueueLock);
		if(!eventQueue.empty()) {
			event = eventQueue.back();
			eventQueue.pop_back();
		}
	}

	if(event) {
		// Handle the event.
		HR_ASSERT_OK(handleEvent(context, event));
	}
	if(!eventQueue.empty()) {
		// Post message to handle next event in the queue.
		// Do not loop to handle all events in the queue
		// so that handling event does not disturb app to process own messages.
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
