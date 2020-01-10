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
		thread->Start();
	}

protected:
	void threadMethod();
	AsyncDispatcher* asyncDispatcher;
};

/**
 * Implementation of tsm::IAsyncDispatcher
 */
class AsyncDispatcher : public tsm::IAsyncDispatcher
{
public:
	virtual HANDLE dispatch(Method method, LPVOID lpParam) override {
		this->method = method;
		this->lpParam = lpParam;
		gcnew ManagedDispatcher(this);

		exitThreadEvent.Attach(CreateEvent(nullptr, TRUE, FALSE, nullptr));
		return exitThreadEvent;
	}

	/**
	 * Method called in the managed thread and call IAsyncDispatcher::Method.
	 */
	void threadMethod() {
		method(lpParam);
		SetEvent(exitThreadEvent);
	}

protected:
	Method method;
	LPVOID lpParam;
	CHandle exitThreadEvent;
};

void ManagedDispatcher::threadMethod()
{
	asyncDispatcher->threadMethod();
}

tsm::IAsyncDispatcher* Context::_createAsyncDispatcher()
{
	return isAsync() ? new AsyncDispatcher() : nullptr;
}

Context::Context(ManagedType^ context, bool isAsync /*= true*/)
	: m_isAsync(isAsync)
	, m_managedContext(context)
	, m_stateMonitor(nullptr)
{
}

State::State(ManagedType^ state, ManagedType^ masterState)
	: m_managedState(state)
	, m_masterState(getNative(masterState))
{
}

State::~State()
{
	delete m_managedState;
}

HRESULT State::_handleEvent(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState)
{
	ManagedType^ _nextState = nullptr;
	auto hr = m_managedState->handleEvent(getManaged((native::Context*)context), getManaged((native::Event*)event), _nextState);
	if(_nextState) {
		*nextState = _nextState->get();
	}
	return (HRESULT)hr;
}

HRESULT State::_entry(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState)
{
	return (HRESULT)m_managedState->entry(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)previousState));
}

HRESULT State::_exit(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState)
{
	return (HRESULT)m_managedState->exit(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)nextState));
}

bool State::_isExitCalledOnShutdown() const
{
	return m_managedState->IsExitCalledOnShutdown;
}

Event::Event(ManagedType^ event)
	: m_managedEvent(event)
	, m_timerClient(nullptr)
{
}

Event::~Event()
{
	delete m_managedEvent;
}

HRESULT Event::_preHandle(tsm::IContext* context)
{
	return (HRESULT)m_managedEvent->preHandle(getManaged((native::Context*)context));
}

HRESULT Event::_postHandle(tsm::IContext* context, HRESULT hr)
{
	return (HRESULT)m_managedEvent->postHandle(getManaged((native::Context*)context), (tsm_NET::HResult)hr);
}
