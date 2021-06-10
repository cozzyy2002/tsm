#pragma once

#include "Unknown.h"

// Suppress compiler warning C4251 at tsm::HandleOwner<T, H>::m_handle member.
#pragma warning(push)
#pragma warning(disable : 4251)

namespace tsm {

tsm_STATE_MACHINE_EXPORT HMODULE GetStateMachineModule();

class IContext;
class IEvent;
class IState;
class IStateMachine;
class IStateMonitor;
class IAsyncDispatcher;

struct ContextHandle;
struct EventHandle;
struct StateHandle;
struct TimerHandle;

struct MByteUnit;

class tsm_STATE_MACHINE_EXPORT ITimerClient
{
public:
	virtual ~ITimerClient() {}

	virtual HRESULT cancelEventTimer(IEvent* event, int timeout = 0) = 0;
	virtual HRESULT cancelAllEventTimers(int timeout = 0) = 0;
	virtual std::vector<CComPtr<IEvent>> getPendingEvents() = 0;

	enum class TimerType {
		None,			// Event is handled ASAP. This value is not used.
		HandleEvent,	// Call StateMachine::handleEvent() when the timer expires.
		TriggerEvent,	// Call StateMachine::triggerEvent() when the timer expires.
	};

	virtual TimerHandle* _getHandle() = 0;
	virtual bool _isHandleCreated() const = 0;
	virtual HRESULT _setEventTimer(TimerType timerType, IContext* context, IEvent* event) = 0;
};

class tsm_STATE_MACHINE_EXPORT ITimerOwner
{
public:
	virtual ITimerClient* _getTimerClient() = 0;

	static ITimerClient* createClient();

	// Short cut for ITimerClient methods.
	HRESULT cancelEventTimer(IEvent* event, int timeout = 0) { return _getTimerClient()->cancelEventTimer(event, timeout); }
	HRESULT cancelAllEventTimers(int timeout = 0) { return _getTimerClient()->cancelAllEventTimers(timeout); }
	std::vector<CComPtr<IEvent>> getPendingEvents() { return _getTimerClient()->getPendingEvents(); }
};

class tsm_STATE_MACHINE_EXPORT IContext : public ITimerOwner
{
public:
	virtual ~IContext() {}

	virtual bool isAsync() const = 0;
	virtual HRESULT getAsyncExitCode(HRESULT* pht) = 0;
	virtual IAsyncDispatcher* _createAsyncDispatcher() = 0;

	virtual IStateMachine* _getStateMachine() = 0;
	virtual IState* _getCurrentState() = 0;
	virtual void _setCurrentState(IState* state) = 0;

	virtual IStateMonitor* _getStateMonitor() = 0;

	virtual ContextHandle* _getHandle(bool reset = false) = 0;

	// Implementation of HandleOwner::_getInstance().
	// Creating ContextHandle depends on value returned by isAsync() method.
	virtual IContext* _getInstance() { return this; }
};

class tsm_STATE_MACHINE_EXPORT IEvent : public Unknown
{
	friend struct TimerHandle;
protected:
	IEvent();

public:
	virtual ~IEvent();

	virtual int _getPriority() const = 0;

#pragma region Methods to be called by StateMachine.
	virtual HRESULT _preHandle(IContext* context) = 0;
	virtual HRESULT _postHandle(IContext* context, HRESULT hr) = 0;
	/**
	 * Compare priority with other IEvent object.
	 * This method is used to queue event by priority order in IStateMachine::triggerEvent().
	 * Implement to return:
	 *   < 0  : Priority is lower than other's.
	 *   == 0 : Priority is same as other's.
	 *   0 <  : Priority is higher than other's.
	 */
	virtual int _comparePriority(IEvent* other) const = 0;
#pragma endregion

#pragma region Definition for event timer
	virtual DWORD _getDelayTime() const = 0;
	virtual DWORD _getIntervalTime() const = 0;
	virtual ITimerClient* _getTimerClient() const = 0;
	virtual int _getTimeoutCount() const = 0;
protected:
	virtual void _setTimeoutCount(int count) = 0;
public:
#pragma endregion

	virtual EventHandle* _getHandle() = 0;

	// Sequence number that indicates creation order.
	// If this method return Zero, it means that overflow occurred.
	long getSequenceNumber() const { return m_sequenceNumber; }

	static void setMemoryWeight(int memoryWeightMByte) { s_memoryWeightMByte = memoryWeightMByte; }
	static int getMemoryWeight() { return s_memoryWeightMByte; }

private:
	static LONG s_sequenceNumber;
	const long m_sequenceNumber;

	static int s_memoryWeightMByte;
	MByteUnit* m_memoryWeight;
};

class tsm_STATE_MACHINE_EXPORT IState : public ITimerOwner, public Unknown
{
protected:
	IState();

public:
	virtual ~IState();

#pragma region Methods to be called by StateMachine.
	virtual HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) = 0;
	virtual HRESULT _entry(IContext* context, IEvent* event, IState* previousState) = 0;
	virtual HRESULT _exit(IContext* context, IEvent* event, IState* nextState) = 0;

	virtual bool _isExitCalledOnShutdown() const = 0;
	virtual IState* _getMasterState() const = 0;
#pragma endregion

	virtual StateHandle* _getHandle() = 0;

	// Sequence number that indicates creation order.
	// If this method return Zero, it means that overflow occurred.
	long getSequenceNumber() const { return m_sequenceNumber; }

	static void setMemoryWeight(int memoryWeightMByte) { s_memoryWeightMByte = memoryWeightMByte; }
	static int getMemoryWeight() { return s_memoryWeightMByte; }

private:
	static LONG s_sequenceNumber;
	const long m_sequenceNumber;

	static int s_memoryWeightMByte;
	MByteUnit* m_memoryWeight;
};

class tsm_STATE_MACHINE_EXPORT IStateMachine
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

/**
 * IAsyncDispatcher interface
 */
class tsm_STATE_MACHINE_EXPORT IAsyncDispatcher
{
public:
	/**
	 * Signature of method to be dispatched.
	 */
	using Method = DWORD(WINAPI*)(LPVOID lpParam);

	/**
	 * dispathc method.
	 *
	 * method: Method to be dispatched.
	 * lpParam: Pointer to parameter to be passed to `method`.
	 * Creates synchronization object handle that is set when the method terminates.
	 * IAsyncDispatcher implementation shooul close this handle on it's destructor.
	 */
	virtual HRESULT dispatch(Method method, LPVOID lpParam, LPHANDLE phWorkerThread = nullptr) = 0;

	/**
	 * Returns exit code of dispatched method.
	 */
	virtual HRESULT getExitCode(HRESULT* phr) = 0;
};

// Monitor interface
class tsm_STATE_MACHINE_EXPORT IStateMonitor
{
public:
	virtual void onIdle(IContext* context) = 0;
	virtual void onEventTriggered(IContext* context, IEvent* event) = 0;
	virtual void onEventHandling(IContext* context, IEvent* event, IState* current) = 0;

	/**
	 * When setup(), previous is nullptr.
	 * When shutdown(), next is nullptr.
	 */
	virtual void onStateChanged(IContext* context, IEvent* event, IState* previous, IState* next) = 0;
	virtual void onTimerStarted(IContext* context, IEvent* event) = 0;
	virtual void onTimerStopped(IContext* context, IEvent* event, HRESULT hr) = 0;
	virtual void onWorkerThreadExit(IContext* context, HRESULT exitCode) = 0;
};

template<class T, class H>
struct tsm_STATE_MACHINE_EXPORT HandleFactory {
	static H* create(T* instance);
	void operator()(H* handle) const;
};

}

#pragma warning(pop)
