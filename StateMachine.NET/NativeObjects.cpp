#include "stdafx.h"
#include "NativeObjects.h"

using namespace native;
using namespace tsm_NET::common;
using namespace System::Diagnostics;

// Dummy difinition to suppress `warning LNK4248: unresolved typeref token`
namespace tsm
{
struct ContextHandle {};
struct StateHandle {};
struct EventHandle {};
struct TimerHandle {};
}

StateMonitor::StateMonitor(StateMonitor::OwnerType^ owner)
	: m_owner(owner)
{
}

void StateMonitor::onIdle(tsm::IContext* context)
{
	m_owner->onIdleCallback(context);
}

void StateMonitor::onEventTriggered(tsm::IContext* context, tsm::IEvent* event)
{
	m_owner->onEventTriggeredCallback(context, event);
}

void StateMonitor::onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
{
	m_owner->onEventHandlingCallback(context, event, current);
}

void StateMonitor::onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
{
	m_owner->onStateChangedCallback(context, event, previous, next);
}

void StateMonitor::onTimerStarted(tsm::IContext* context, tsm::IEvent* event)
{
	m_owner->onTimerStartedCallback(context, event);
}

void StateMonitor::onTimerStopped(tsm::IContext* context, tsm::IEvent* event, HRESULT hr)
{
	m_owner->onTimerStoppedCallback(context, event, hr);
}

void StateMonitor::onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode)
{
	m_owner->onWorkerThreadExitCallback(context, exitCode);
}

class AsyncDispatcher;

/**
 * Managed class to dispatch tsm::IAsyncDispatcher::Method(lpParam) in managed worker thread.
 */
ref class ManagedDispatcher
{
public:
	ManagedDispatcher(AsyncDispatcher* asyncDispatcher)
		: asyncDispatcher(asyncDispatcher)
	{
		auto threadStart = gcnew Threading::ThreadStart(this, &ManagedDispatcher::threadMethod);
		auto thread = gcnew Threading::Thread(threadStart);
		 threadID++;
		thread->Name = threadID.ToString();
		thread->Start();
	}

protected:
	void threadMethod();
	AsyncDispatcher* asyncDispatcher;
	static int threadID = 0;
};

/**
 * Implementation of tsm::IAsyncDispatcher
 */
class AsyncDispatcher : public tsm::IAsyncDispatcher
{
public:
	virtual HRESULT dispatch(Method method, LPVOID lpParam, LPHANDLE phWorkerThread) override {
		if(!method) { return E_POINTER; }

		HRESULT hr = S_OK;
		if(phWorkerThread) {
			auto h = CreateEvent(nullptr, TRUE, FALSE, nullptr);
			if(h) {
				// Create event object which notifies that the worker thread terminates.
				exitThreadEvent.Attach(h);
				*phWorkerThread = h;
			} else {
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
		}

		// Dispatch threadMethod on managed thread.
		this->method = method;
		this->lpParam = lpParam;
		gcnew ManagedDispatcher(this);
		return hr;
	}

	/**
	 * Method called in the managed thread and call IAsyncDispatcher::Method.
	 */
	void threadMethod() {
		exitCode = method(lpParam);
		if(exitThreadEvent) { SetEvent(exitThreadEvent); }
	}

	virtual HRESULT getExitCode(HRESULT* phr) override {
		HRESULT hr = S_OK;
		if(!phr) { return E_POINTER; }
		if(!exitThreadEvent) { return E_ILLEGAL_METHOD_CALL; }

		auto w = WaitForSingleObject(exitThreadEvent, 0);
		switch(w) {
		case WAIT_OBJECT_0:
			// Worker thread has been terminated.
			*phr = exitCode;
			break;
		case WAIT_TIMEOUT:
			// Worker thread is still running.
			hr = E_ILLEGAL_METHOD_CALL;
			break;
		case WAIT_FAILED:
			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		default:
			hr = E_UNEXPECTED;
			break;
		}

		return hr;
	}

protected:
	Method method;
	LPVOID lpParam;
	DWORD exitCode;
	CHandle exitThreadEvent;
};

void ManagedDispatcher::threadMethod()
{
	asyncDispatcher->threadMethod();
}

tsm::IAsyncDispatcher* Context::_createAsyncDispatcher()
{
	return m_managedContext->UseNativeThread ? new AsyncDispatcher() : tsm::Context_createAsyncDispatcher();
}

Context::Context(ManagedType^ context, bool isAsync /*= true*/)
	: m_isAsync(isAsync)
	, m_managedContext(context)
	, m_stateMonitor(nullptr)
{
}

HRESULT Context::getAsyncExitCode(HRESULT* pht)
{
	return tsm::Context_getAsyncExitCode(this, pht);
}

State::State(ManagedType^ state, ManagedType^ masterState, bool autoDispose)
	: m_managedState(state)
	, m_autoDispose(autoDispose)
{
}

State::~State()
{
	if(m_autoDispose) {
		// Delete managed object automatically.
		// In the case of m_autoDispose == false, See Managed State constructor and finalizer.
		delete m_managedState;
	}
}

HRESULT State::handleEvent(Context* context, Event* event, State** nextState)
{
	ManagedType^ _nextState = nullptr;
	auto hr = m_managedState->_handleEvent(getManaged((native::Context*)context), getManaged((native::Event*)event), _nextState);
	if(_nextState) {
		*nextState = (State*)_nextState->get();
	}
	return (HRESULT)hr;
}

HRESULT State::entry(Context* context, Event* event, State* previousState)
{
	return (HRESULT)m_managedState->_entry(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)previousState));
}

HRESULT State::exit(Context* context, Event* event, State* nextState)
{
	return (HRESULT)m_managedState->_exit(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)nextState));
}

bool State::_isExitCalledOnShutdown() const
{
	return m_managedState->IsExitCalledOnShutdown;
}

Event::Event(ManagedType^ event, int priority, bool autoDispose)
	: m_managedEvent(event)
	, m_autoDispose(autoDispose)
{
}

Event::~Event()
{
	if(m_autoDispose) {
		// When reference count of this object, Managed object is also deleted.
		// In the case of m_autoDispose == false, See Managed Event constructor and finalizer.
		delete m_managedEvent;
	}
}

HRESULT Event::preHandle(Context* context)
{
	return (HRESULT)m_managedEvent->_preHandle(getManaged((native::Context*)context));
}

HRESULT Event::postHandle(Context* context, HRESULT hr)
{
	return (HRESULT)m_managedEvent->_postHandle(getManaged((native::Context*)context), (tsm_NET::HResult)hr);
}
