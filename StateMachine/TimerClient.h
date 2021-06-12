#pragma once

#include <StateMachine/Interface.h>

// Suppress compiler warning C4251 at tsm::HandleOwner<T, H>::m_handle member in interface.h.
#pragma warning(push)
#pragma warning(disable : 4251)

namespace tsm {

struct TimerHandle;

class TimerClient : public ITimerClient
{
public:
	HRESULT cancelEventTimer(IEvent* event, int timeout = 0) override;
	HRESULT cancelAllEventTimers(int timeout = 0) override;
	std::vector<CComPtr<IEvent>> getPendingEvents() override;

	TimerHandle* _getHandle() override {
		if(!m_handle) {
			m_handle.reset(HandleFactory<TimerClient, TimerHandle>::create(this));
		}
		return m_handle.get();
	}

	HRESULT _setEventTimer(TimerType timerType, IContext* context, IEvent* event) override;
	bool _isHandleCreated() const { return m_handle ? true : false; }

protected:
	std::unique_ptr<TimerHandle, HandleFactory<TimerClient, TimerHandle>> m_handle;
};

}

#pragma warning(pop)
