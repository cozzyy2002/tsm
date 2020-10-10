#include <StateMachine/stdafx.h>

#include <StateMachine/TimerClient.h>
#include <StateMachine/Assert.h>

#include "Handles.h"

namespace tsm {

static DWORD WINAPI timerThread(LPVOID lpParam);

// Cancel timer.
HRESULT TimerHandle::Timer::cancel(DWORD timeout)
{
	WIN32_ASSERT(SetEvent(canceledEvent));
	auto hr = S_OK;
	if(0 < timeout) {
		auto wait = WaitForSingleObject(terminatedEvent, timeout);
		switch(wait) {
		case WAIT_OBJECT_0:
			// Timer is successfully canceled.
			break;
		case WAIT_TIMEOUT:
			// User method called by timer might be executing.
			hr = E_ABORT;
			break;
		case WAIT_FAILED:
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		default:
			hr = E_UNEXPECTED;
			break;
		}
	}
	return hr;
}

HRESULT TimerClient::cancelEventTimer(IEvent* event, int timeout /*= 0*/)
{
	if(!_isHandleCreated()) { return S_FALSE; }

	auto th = _getHandle();

	auto hr = S_OK;
	{
		// Preserve Timer object until callback will complete or will be canceled.
		CComPtr<TimerHandle::Timer> timer;
		{
			lock_t _lock(th->lock);

			auto it = th->timers.find(event);
			if(it != th->timers.end()) {
				// Cancel and remove timer.
				timer = it->second;
				th->timers.erase(it);
			} else {
				// Timer specified might have expired already.
				hr = S_FALSE;
			}
		}
		if(timer) { hr = HR_EXPECT_OK(timer->cancel(timeout)); }
	}
	return hr;
}

HRESULT TimerClient::cancelAllEventTimers(int timeout /*= 0*/)
{
	if(!_isHandleCreated()) { return S_FALSE; }

	auto hr = S_OK;
	auto th = _getHandle();

	lock_t _lock(th->lock);
	if(th->timers.empty()) { return S_FALSE; }

	// Cancel and delete all pending timers.
	for(auto& pair : th->timers) {
		auto _hr = HR_EXPECT_OK(pair.second->cancel(timeout));
		// Even if canceling a timer fails, continue canceling all other timers.
		if(FAILED(_hr)) { hr = _hr; }
	}
	th->timers.clear();

	return hr;
}

std::vector<CComPtr<IEvent>> TimerClient::getPendingEvents()
{
	std::vector<CComPtr<IEvent>> events;

	if(_isHandleCreated()) {
		auto th = _getHandle();
		lock_t _lock(th->lock);

		for(auto& pair : th->timers) {
			events.push_back(pair.first);
		}
	}
	return events;
}

HRESULT TimerClient::_setEventTimer(TimerType timerType, IContext* context, IEvent* event)
{
	// Ensure to release object on error.
	CComPtr<IEvent> _event(event);

	// Ensure that timer is required.
	HR_ASSERT(event->_getTimerClient(), E_ILLEGAL_METHOD_CALL);

	// State machine should not call this method with event whose timer is created already.
	auto eh = event->_getHandle();
	HR_ASSERT(!eh->isTimerCreated, E_ILLEGAL_METHOD_CALL);
	eh->isTimerCreated = true;

	// Create timer object.
	CComPtr<TimerHandle::Timer> timer(new TimerHandle::Timer());
	timer->timerType = timerType;
	timer->context = context;
	timer->event = event;
	timer->canceledEvent.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));

	{
		// Add Event and Timer pair to TimerHandle::timers.
		auto th = _getHandle();
		lock_t _lock(th->lock);
		th->timers.insert(std::make_pair(event, timer));
	}

	// Create IAsyncDispatcher and dispatch timer thread with timer object as it's parameter.
	timer->asyncDispatcher.reset(context->_createAsyncDispatcher());
	HR_ASSERT(timer->asyncDispatcher, E_UNEXPECTED);
	HR_ASSERT_OK(timer->asyncDispatcher->dispatch(timerThread, timer.p, &timer->terminatedEvent));

	return S_OK;
}

// Wait delay time and interval time, then call TimerHandle::timerThread() method.
/*static*/ DWORD WINAPI timerThread(LPVOID lpParam)
{
	CComPtr<TimerHandle::Timer> timer = (TimerHandle::Timer*)lpParam;
	auto event = timer->event.p;
	auto timerClient = event->_getTimerClient();
	auto th = timerClient->_getHandle();

	auto context = timer->context;
	context->_getHandle()->callStateMonitor(context, [event](IContext* context, IStateMonitor* stateMonitor)
	{
		stateMonitor->onTimerStarted(context, event);
	});

	auto hr = th->timerThread(timer);

	// Clear the event of the timer if exists.
	{
		lock_t _lock(th->lock);
		auto it = th->timers.find(event);
		if(it != th->timers.end()) { th->timers.erase(it); }
	}

	context->_getHandle()->callStateMonitor(context, [event, hr](IContext* context, IStateMonitor* stateMonitor)
	{
		stateMonitor->onTimerStopped(context, event, hr);
	});

	return 0;
}

HRESULT TimerHandle::timerThread(TimerHandle::Timer* timer)
{
	auto event = timer->event.p;
	auto timerClient = event->_getTimerClient();

	// Wait delay time.
	DWORD wait = WAIT_TIMEOUT;
	auto delay = event->_getDelayTime();
	if(delay) {
		wait = WaitForSingleObject(timer->canceledEvent, delay);
		switch(wait) {
		case WAIT_TIMEOUT:
			HR_ASSERT_OK(timerCallback(timer, event));
			break;
		case WAIT_OBJECT_0:
			return S_OK;
		case WAIT_FAILED:
			return HRESULT_FROM_WIN32(GetLastError());
		default:
			return E_UNEXPECTED;
		}
	}

	// Wait interval time until timer will be canceled.
	auto interval = event->_getIntervalTime();
	if(interval) {
		while(true) {
			wait = WaitForSingleObject(timer->canceledEvent, interval);
			switch(wait) {
			case WAIT_TIMEOUT:
				HR_ASSERT_OK(timerCallback(timer, event));
				break;
			case WAIT_OBJECT_0:
				return S_OK;
			case WAIT_FAILED:
				return HRESULT_FROM_WIN32(GetLastError());
			default:
				return E_UNEXPECTED;
			}
		}
	}
	return 0;
}

HRESULT TimerHandle::timerCallback(TimerHandle::Timer* timer, IEvent* event)
{
	{
		lock_t _lock(lock);

		auto it = timers.find(event);
		if(it != timers.end()) {
			if(event->_getIntervalTime() == 0) {
				// Delete timer except for interval timer.
				timers.erase(it);
			}
		} else {
			// The timer has been canceled.
			return S_FALSE;
		}
	}

	auto stateMachine = timer->context->_getStateMachine();
	switch(timer->timerType) {
	case TimerClient::TimerType::HandleEvent:
		HR_ASSERT_OK(stateMachine->handleEvent(timer->context, event));
		break;
	case TimerClient::TimerType::TriggerEvent:
		HR_ASSERT_OK(stateMachine->triggerEvent(timer->context, event));
		break;
	}
	return S_OK;
}

}
