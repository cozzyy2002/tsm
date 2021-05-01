#pragma once

#include <StateMachine/Interface.h>

#include <vector>

// Suppress compiler warning C4251 at tsm::HandleOwner<T, H>::m_handle member in interface.h.
#pragma warning(push)
#pragma warning(disable : 4251)

namespace tsm {

struct TimerHandle;

class tsm_STATE_MACHINE_EXPORT TimerClient
{
public:
	HRESULT cancelEventTimer(IEvent* event, int timeout = 0);
	HRESULT cancelAllEventTimers(int timeout = 0);
	std::vector<CComPtr<IEvent>> getPendingEvents();

	enum class TimerType {
		None,			// Event is handled ASAP. This value is not used.
		HandleEvent,	// Call StateMachine::handleEvent() when the timer expires.
		TriggerEvent,	// Call StateMachine::triggerEvent() when the timer expires.
	};
	HRESULT _setEventTimer(TimerType timerType, IContext* context, IEvent* event);

	TimerHandle* _getHandle() {
		if(!m_handle) {
			m_handle.reset(HandleFactory<TimerClient, TimerHandle>::create(this));
		}
		return m_handle.get();
	}
	bool _isHandleCreated() const { return m_handle ? true : false; }

protected:
	std::unique_ptr<TimerHandle, HandleFactory<TimerClient, TimerHandle>> m_handle;
};

}

#pragma warning(pop)
