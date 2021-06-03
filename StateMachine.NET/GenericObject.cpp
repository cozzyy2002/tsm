#include "stdafx.h"
#include "GenericObject.h"
#include "NativeObjects.h"

namespace tsm_NET
{
using namespace common;

namespace Generic
{
	static IList<IEvent^>^ getPendingEvents(ITimerOwner^ timerOwner)
	{
		auto ret = gcnew List<IEvent^>();
		auto timerClient = timerOwner->getTimerClient();
		for(auto e : timerClient->getPendingEvents()) {
			ret->Add(getManaged((native::Event*)e.p));
		}
		return ret;
	}

	generic<typename E, typename S>
	StateMonitorCaller<E, S>::StateMonitorCaller(IStateMonitor<E, S>^ stateMonitor)
		: tsm_NET::StateMonitorCaller(nullptr)
		, m_stateMonitor(stateMonitor)
	{
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onIdleCallback(tsm::IContext* context)
	{
		m_stateMonitor->onIdle((Context<E, S>^)getManaged((native::Context*)context));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onEventTriggeredCallback(tsm::IContext* context, tsm::IEvent* event)
	{
		m_stateMonitor->onEventTriggered((Context<E, S>^)getManaged((native::Context*)context), (E)getManaged((native::Event*)event));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onEventHandlingCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
	{
		m_stateMonitor->onEventHandling((Context<E, S>^)getManaged((native::Context*)context), (E)getManaged((native::Event*)event), (S)getManaged((native::State*)current));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onStateChangedCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
	{
		m_stateMonitor->onStateChanged((Context<E, S>^)getManaged((native::Context*)context), (E)getManaged((native::Event*)event), (S)getManaged((native::State*)previous), (S)getManaged((native::State*)next));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onTimerStartedCallback(tsm::IContext* context, tsm::IEvent* event)
	{
		m_stateMonitor->onTimerStarted((Context<E, S>^)getManaged((native::Context*)context), (E)getManaged((native::Event*)event));
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onTimerStoppedCallback(tsm::IContext* context, tsm::IEvent* event, HRESULT hr)
	{
		m_stateMonitor->onTimerStopped((Context<E, S>^)getManaged((native::Context*)context), (E)getManaged((native::Event*)event), (HResult)hr);
	}

	generic<typename E, typename S>
	void StateMonitorCaller<E, S>::onWorkerThreadExitCallback(tsm::IContext* context, HRESULT exitCode)
	{
		m_stateMonitor->onWorkerThreadExit((Context<E, S>^)getManaged((native::Context*)context), (HResult)exitCode);
	}

	generic<typename E, typename S>
	void Context<E, S>::construct(bool isAsync, bool useNativeThread)
	{
		m_useNativeThread = useNativeThread;
		m_nativeContext = new native::Context(this, isAsync);
	}

	generic<typename E, typename S>
	HResult Context<E, S>::setup(S initialState, E event)
	{
		return (HResult)m_nativeContext->setup(getNative((IState^)initialState), getNative((IEvent^)event));
	}

	generic<typename E, typename S>
	HResult Context<E, S>::setup(S initialState)
	{
		return (HResult)m_nativeContext->setup(getNative((IState^)initialState));
	}

	generic<typename E, typename S>
	HResult Context<E, S>::shutdown(TimeSpan timeout)
	{
		return (HResult)m_nativeContext->shutdown((DWORD)timeout.TotalMilliseconds);
	}

	generic<typename E, typename S>
	HResult Context<E, S>::shutdown(int timeout_msec)
	{
		return (HResult)m_nativeContext->shutdown((DWORD)timeout_msec);
	}

	generic<typename E, typename S>
	HResult Context<E, S>::shutdown()
	{
		return (HResult)m_nativeContext->shutdown();
	}

	generic<typename E, typename S>
	HResult Context<E, S>::triggerEvent(E event)
	{
		return (HResult)m_nativeContext->triggerEvent(event->get());
	}

	generic<typename E, typename S>
	HResult Context<E, S>::handleEvent(E event)
	{
		return (HResult)m_nativeContext->handleEvent(event->get());
	}

	generic<typename E, typename S>
	HResult Context<E, S>::waitReady(TimeSpan timeout)
	{
		return (HResult)m_nativeContext->waitReady((DWORD)timeout.TotalMilliseconds);
	}

	generic<typename E, typename S>
	HResult Context<E, S>::waitReady(int timeout_msec)
	{
		return (HResult)m_nativeContext->waitReady(timeout_msec);
	}

	generic<typename E, typename S>
	bool Context<E, S>::IsAsync::get()
	{
		return m_nativeContext->isAsync();
	}

	generic<typename E, typename S>
	S Context<E, S>::getCurrentState()
	{
		auto state = m_nativeContext->getCurrentState();
		return (S)(state ? state->get() : nullptr);
	}

	generic<typename E, typename S>
	IList<IEvent^>^ Context<E, S>::PendingEvents::get()
	{
		return getPendingEvents(this);
	}

	generic<typename E, typename S>
	tsm::ITimerClient* Context<E, S>::getTimerClient()
	{
		return m_nativeContext->_getTimerClient();
	}

	generic<typename E, typename S>
	void Context<E, S>::StateMonitor::set(IStateMonitor<E, S>^ value)
	{
		m_stateMonitor = value;
		if(value) {
			m_stateMonitorCaller = gcnew StateMonitorCaller<E, S>(value);
			m_nativeContext->setStateMonitor(m_stateMonitorCaller->get());
		} else {
			m_nativeContext->setStateMonitor(nullptr);
			delete m_stateMonitorCaller;
			m_stateMonitorCaller = nullptr;
		}
	}

	generic<typename E, typename S>
	HResult AsyncContext<E, S>::getAsyncExitCode([Out] HResult% hrExitCode)
	{
		HRESULT _hrExitCode;
		auto hr = tsm_NET::getAsyncExitCode(m_nativeContext, &_hrExitCode);
		if(SUCCEEDED(hr)) { hrExitCode = (HResult)_hrExitCode; }
		return (HResult)hr;
	}

	generic<typename C, typename E, typename S>
	void State<C, E, S>::construct(S masterState, bool autoDispose)
	{
		m_nativeState = new native::State(this, masterState, autoDispose);
		if(!autoDispose) {
			// Prevent native object from being deleted automatically.
			m_nativeState->AddRef();
		}

		IsExitCalledOnShutdown = false;
	}

	generic<typename C, typename E, typename S>
	State<C, E, S>::~State()
	{
		this->!State();
	}

	generic<typename C, typename E, typename S>
	State<C, E, S>::!State()
	{
		if(m_nativeState && !m_nativeState->m_autoDispose) {
			m_nativeState->Release();
		}
		m_nativeState = nullptr;
	}

	generic<typename C, typename E, typename S>
	HResult State<C, E, S>::_handleEvent(tsm_NET::IContext^ context, tsm_NET::IEvent^ event, tsm_NET::IState^% nextState)
	{
		S _nextState;
		auto hr = handleEvent((C)context, (E)event, _nextState);
		if(_nextState) {
			nextState = _nextState;
		}
		return (tsm_NET::HResult)hr;
	}

	generic<typename C, typename E, typename S>
	HResult State<C, E, S>::_entry(tsm_NET::IContext^ context, tsm_NET::IEvent^ event, tsm_NET::IState^ previousState)
	{
		return (tsm_NET::HResult)entry((C)context, (E)event, (S)previousState);
	}

	generic<typename C, typename E, typename S>
	HResult State<C, E, S>::_exit(tsm_NET::IContext^ context, tsm_NET::IEvent^ event, tsm_NET::IState^ nextState)
	{
		return (tsm_NET::HResult)exit((C)context, (E)event, (S)nextState);
	}

	generic<typename C, typename E, typename S>
	bool State<C, E, S>::IsSubState::get()
	{
		return m_nativeState->isSubState();
	}

	generic<typename C, typename E, typename S>
	S State<C, E, S>::getMasterState()
	{
		auto state = m_nativeState->getMasterState();
		return (S)(state ? state->get() : nullptr);
	}

	generic<typename C, typename E, typename S>
	bool State<C, E, S>::AutoDispose::get()
	{
		return m_nativeState->m_autoDispose;
	}

	generic<typename C, typename E, typename S>
	long State<C, E, S>::SequenceNumber::get()
	{
		return m_nativeState->getSequenceNumber();
	}

	generic<typename C, typename E, typename S>
	int State<C, E, S>::MemoryWeight::get()
	{
		return tsm::IState::getMemoryWeight();
	}

	generic<typename C, typename E, typename S>
	void State<C, E, S>::MemoryWeight::set(int value)
	{
		tsm::IState::setMemoryWeight(value);
	}

	generic<typename C, typename E, typename S>
	IList<IEvent^>^ State<C, E, S>::PendingEvents::get()
	{
		return getPendingEvents(this);
	}

	generic<typename C, typename E, typename S>
	tsm::ITimerClient* State<C, E, S>::getTimerClient()
	{
		return m_nativeState->_getTimerClient();
	}

	generic<typename C>
	void Event<C>::construct(int priority, bool autoDispose)
	{
		m_nativeEvent = new native::Event(this, priority, autoDispose);
		if(autoDispose) {
			// Prevent native object from being deleted automatically.
			m_nativeEvent->AddRef();
		}
	}

	generic<typename C>
	Event<C>::~Event()
	{
		this->!Event();
	}

	generic<typename C>
	Event<C>::!Event()
	{
		if(m_nativeEvent && !m_nativeEvent->m_autoDispose) {
			m_nativeEvent->Release();
		}
		m_nativeEvent = nullptr;
	}

	generic<typename C>
	HResult Event<C>::_preHandle(tsm_NET::IContext^ context)
	{
		return (tsm_NET::HResult)preHandle((C)context);
	}

	generic<typename C>
	HResult Event<C>::_postHandle(tsm_NET::IContext^ context, HResult hr)
	{
		return (tsm_NET::HResult)postHandle((C)context, (HResult)hr);
	}

	generic<typename C>
	void Event<C>::setTimer(tsm_NET::ITimerOwner^ timerOwner, int delayTime_msec, int intervalTime_msec)
	{
		m_nativeEvent->setTimer(timerOwner->getTimerClient(), delayTime_msec, intervalTime_msec);
	}

	generic<typename C>
	TimeSpan Event<C>::DelayTime::get()
	{
		return TimeSpan::FromMilliseconds(m_nativeEvent->_getDelayTime());
	}

	generic<typename C>
	TimeSpan Event<C>::IntervalTime::get()
	{
		return TimeSpan::FromMilliseconds(m_nativeEvent->_getIntervalTime());
	}
	
	generic<typename C>
	int Event<C>::TimeoutCount::get()
	{
		return m_nativeEvent->_getTimeoutCount();
	}
	
	generic<typename C>
	int Event<C>::Priority::get()
	{
		return m_nativeEvent->_getPriority();
	}

	generic<typename C>
	HResult Event<C>::cancelTimer()
	{
		return (HResult)m_nativeEvent->cancelTimer();
	}

	generic<typename C>
	HResult Event<C>::cancelTimer(TimeSpan timeout)
	{
		return (HResult)m_nativeEvent->cancelTimer((int)timeout.TotalMilliseconds);
	}

	generic<typename C>
	HResult Event<C>::cancelTimer(int timeout)
	{
		return (HResult)m_nativeEvent->cancelTimer(timeout);
	}

	generic<typename C>
	bool Event<C>::AutoDispose::get()
	{
		return m_nativeEvent->m_autoDispose;
	}

	generic<typename C>
	long Event<C>::SequenceNumber::get()
	{
		return m_nativeEvent->getSequenceNumber();
	}

	generic<typename C>
	int Event<C>::MemoryWeight::get()
	{
		return tsm::IEvent::getMemoryWeight();
	}

	generic<typename C>
	void Event<C>::MemoryWeight::set(int value)
	{
		tsm::IEvent::setMemoryWeight(value);
	}
}

HRESULT tsm_NET::getAsyncExitCode(native::Context* context, HRESULT* phr)
{
	return context->getAsyncExitCode(phr);
}
}
