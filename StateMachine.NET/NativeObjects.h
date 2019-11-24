#pragma once

#include "StateMachine.NET.h"

using System::Runtime::InteropServices::Marshal;

namespace native
{
class Context;
class State;
class Event;

/*
	Callback template class.
	This class is used to avoid `Cannot pass a GCHandle across AppDomains` exception
	when method of managed class is called by native(unmanaged) class in worker thread.

	How to use this class.

	1. Define delegate(class D), callback signature(class C) and callback method in managed class(class M).
		delegate HRESULT XxxDelegate(...);				// delegate
		typedef HRESULT (__stdcall *XxxCallback)(...);	// callback signature
		HRESULT xxxCallback(...) { method body }		// callback method

	2. Declare member variable of this class in native class.
		Callback<ManagedClass, XxxDelegate, XxxCallback> m_xxxCallback;

	3. Initialize the member variable in the constructor of native class.
		m_xxxCallback(managedObject, gcnew XxxDelegate(managedObject, &ManagedClass::xxxCallback))

	4. Callback from native class.
		m_xxxCallback(...);

	NOTE:
		Parameters and return value should be native(unmanaged) type.

	See http://lambert.geek.nz/2007/05/unmanaged-appdomain-callback/.
*/
template<class M, class D, class C>
class Callback
{
public:
	Callback(M^ managed, D^ del) : del(del) {
		callback = (C)Marshal::GetFunctionPointerForDelegate(del).ToPointer();
	}

	operator C() { return callback; }

protected:
	gcroot<D^> del;
	C callback;
};

class StateMonitor : public tsm::IStateMonitor
{
public:
	using ManagedType = tsm_NET::IStateMonitor;
	using Context = tsm_NET::Context;

	StateMonitor(ManagedType^ stateMonitor, Context^ context);

	virtual void onIdle(tsm::IContext* context) override;
	virtual void onEventTriggered(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current) override;

	/**
	 * When setup(), previous is nullptr.
	 * When shutdown(), next is nullptr.
	 */
	virtual void onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next) override;
	virtual void onTimerStarted(tsm::IContext* context, tsm::IEvent* event) override {}
	virtual void onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode) override {}

protected:
	gcroot<ManagedType^> m_managedStateMonitor;

	Callback<ManagedType, Context::OnIdleDelegate, Context::OnIdleCallback> m_onIdleCallback;
	Callback<ManagedType, Context::OnEventTriggeredDelegate, Context::OnEventTriggeredCallback> m_onEventTriggeredCallback;
	Callback<ManagedType, Context::OnEventHandlingDelegate, Context::OnEventHandlingCallback> m_onEventHandlingCallback;
	Callback<ManagedType, Context::OnStateChangedDelegate, Context::OnStateChangedCallback> m_onStateChangedCallback;
};

class Context : public tsm::IContext, public tsm::TimerClient
{
public:
	using ManagedType = tsm_NET::Context;

	Context(ManagedType^ context, bool isAsync = true);
	virtual bool isAsync() const override { return m_isAsync; }

	HRESULT setup(tsm::IState* initialState, tsm::IEvent* event = nullptr) { return _getStateMachine()->setup(this, initialState, event); }
	HRESULT shutdown(DWORD timeout = 100) { return _getStateMachine()->shutdown(this, timeout); }
	HRESULT triggerEvent(tsm::IEvent* event) { return _getStateMachine()->triggerEvent(this, event); }
	HRESULT handleEvent(tsm::IEvent* event) { return _getStateMachine()->handleEvent(this, event); }
	HRESULT waitReady(DWORD timeout = 100) { return _getStateMachine()->waitReady(this, timeout); }

	virtual tsm::IStateMachine* _getStateMachine() override {
		if(!m_stateMachine) m_stateMachine.reset(tsm::IStateMachine::create(this));
		return m_stateMachine.get();
	}
	virtual tsm::IState* _getCurrentState() override { return m_currentState; }
	virtual void _setCurrentState(tsm::IState* state) override { m_currentState = state; }

	virtual tsm::IStateMonitor* _getStateMonitor() override { return m_stateMonitor.get(); }
	void setStateMonitor(tsm_NET::IStateMonitor^ value);

	// Implementation of IContext::_getTimerClient().
	virtual TimerClient* _getTimerClient() override { return this; }

	ManagedType^ get() { return m_managedContext; }

	inline State* getCurrentState() { return (State*)_getCurrentState(); }

protected:
	gcroot<ManagedType^> m_managedContext;
	bool m_isAsync;

	std::unique_ptr<tsm::IStateMachine> m_stateMachine;
	CComPtr<tsm::IState> m_currentState;

	std::unique_ptr<StateMonitor> m_stateMonitor;
};

class State : public tsm::IState, public tsm::TimerClient
{
public:
	using ManagedType = tsm_NET::State;

	State(ManagedType^ state, ManagedType^ masterState);

#pragma region Implementation of IState that call methods of managed class.
	virtual HRESULT _handleEvent(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState) override;
	virtual HRESULT _entry(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState) override;
	virtual HRESULT _exit(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState) override;

	virtual bool _callExitOnShutdown() const override { return false; }
	virtual IState* _getMasterState() const override { return m_masterState; }

	virtual TimerClient* _getTimerClient() override { return this; }
#pragma endregion

	inline State* getMasterState() const { return (State*)_getMasterState(); }
	bool isSubState() const { return m_masterState ? true : false; }

	ManagedType^ get() { return m_managedState; }

protected:
	gcroot<ManagedType^> m_managedState;

	Callback<ManagedType, ManagedType::HandleEventDelegate, ManagedType::HandleEventCallback> m_handleEventCallback;
	Callback<ManagedType, ManagedType::EntryDelegate, ManagedType::EntryCallback> m_entryCallback;
	Callback<ManagedType, ManagedType::ExitDelegate, ManagedType::ExitCallback> m_exitCallback;

	CComPtr<State> m_masterState;
};

class Event : public tsm::IEvent
{
public:
	using ManagedType = tsm_NET::Event;

	Event(ManagedType^ event);

#pragma region Implementation of IState that call methods of managed class.
	virtual HRESULT _preHandle(tsm::IContext* Icontext) override;
	virtual HRESULT _postHandle(tsm::IContext* Icontext, HRESULT hr) override;
#pragma endregion

	virtual int _getPriority() const override { return m_priority; }
	virtual DWORD _getDelayTime() const override { return m_delayTime; }
	virtual DWORD _getIntervalTime() const override { return m_intervalTime; }
	virtual tsm::TimerClient* _getTimerClient() const override { return m_timerClient; }

	ManagedType^ get() { return m_managedEvent; }

protected:
	gcroot<ManagedType^> m_managedEvent;

	Callback<ManagedType, ManagedType::PreHandleDelegate, ManagedType::PreHandleCallback> m_preHandleCallback;
	Callback<ManagedType, ManagedType::PostHandleDelegate, ManagedType::PostHandleCallback> m_postHandleCallback;

	int m_priority;
	DWORD m_delayTime;
	DWORD m_intervalTime;
	tsm::TimerClient* m_timerClient;
};
}