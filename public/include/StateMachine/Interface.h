#pragma once

#include "Unknown.h"

#include <mutex>

namespace tsm {

class IEvent;
class IState;
class IStateMachine;
class IStateMonitor;
class TimerClient;

struct EventHandle;
struct StateHandle;
struct ContextHandle;

using lock_object_t = std::recursive_mutex;
using lock_t = std::lock_guard<lock_object_t>;

template<class T, class H>
class HandleOwner
{
public:
	virtual H* _getHandle() {
		if(!m_handle) m_handle.reset(HandleFactory::create(_getInstance()));
		return m_handle.get();
	}

protected:
	// Returns sub class instance.
	// Override if constructor of handle class depends on the instance.
	virtual T* _getInstance() { return nullptr; }

	struct HandleFactory {
		static H* create(T* instance);
		void operator()(H* handle) const;
	};

	std::unique_ptr<H, HandleFactory> m_handle;
};

class IContext : public HandleOwner<IContext, ContextHandle>
{
public:
	virtual ~IContext() {}

	virtual bool isAsync() const = 0;

	virtual IStateMachine* _getStateMachine() = 0;
	virtual IState* _getCurrentState() = 0;
	virtual void _setCurrentState(IState* state) = 0;

	using OnAssertFailed = void(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);
	static OnAssertFailed* onAssertFailedProc;

	virtual lock_t* _getHandleEventLock() = 0;

	virtual IStateMonitor* _getStateMonitor() = 0;

	// Returns TimerClient instance.
	// Sub class may returns this pointer.
	virtual TimerClient* _getTimerClient() = 0;

	// Implementation of HandleOwner::_getInstance().
	// Creating ContextHandle depends on value returned by isAsync() method.
	virtual IContext* _getInstance() override { return this; }
};

class IEvent : public HandleOwner<IEvent, EventHandle>, public Unknown
{
public:
	virtual ~IEvent() {}

	virtual int _getPriority() const = 0;

#pragma region Definition for event timer
	virtual DWORD _getDelayTime() const = 0;
	virtual DWORD _getIntervalTime() const = 0;
	virtual TimerClient* _getTimerClient() const = 0;
#pragma endregion
};

class IState : public HandleOwner<IState, StateHandle>, public Unknown
{
public:
	virtual ~IState() {}

#pragma region Methods to be called by StateMachine.
	virtual HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) = 0;
	virtual HRESULT _entry(IContext* context, IEvent* event, IState* previousState) = 0;
	virtual HRESULT _exit(IContext* context, IEvent* event, IState* nextState) = 0;

	virtual bool _callExitOnShutdown() const = 0;
	virtual IState* _getMasterState() const = 0;
	virtual void _setMasterState(IState* state) = 0;
#pragma endregion

	// Returns TimerClient instance.
	// Sub class may returns this pointer.
	virtual TimerClient* _getTimerClient() = 0;
};

class IStateMachine
{
public:
	static IStateMachine* create(IContext* context);
	static IStateMachine* create(HWND hWnd, UINT msg);

	virtual ~IStateMachine() {}

	virtual HRESULT setup(IContext* context, IState* initialState, IEvent* event) = 0;
	virtual HRESULT shutdown(IContext* context, DWORD timeout) = 0;
	virtual HRESULT triggerEvent(IContext* context, IEvent* event) = 0;
	virtual HRESULT handleEvent(IContext* context, IEvent* event) = 0;
	virtual HRESULT waitReady(IContext* context, DWORD timeout) = 0;
};

// Monitor interface
class IStateMonitor
{
public:
	virtual void onIdle(IContext* context) = 0;
	virtual void onEventTriggered(IContext* context, IEvent* event) = 0;
	virtual void onEventHandling(IContext* context, IEvent* event, IState* current) = 0;
	virtual void onStateChanged(IContext* context, IEvent* event, IState* previous, IState* next) = 0;
	virtual void onTimerStarted(IContext* context, IEvent* event) = 0;
	virtual void onWorkerThreadExit(IContext* context, HRESULT exitCode) = 0;
};

}
