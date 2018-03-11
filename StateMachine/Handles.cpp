#include <StateMachine/stdafx.h>

#include "Handles.h"

namespace tsm {

EventHandle* HandleOwner<IEvent, EventHandle>::_getHandle()
{
	if(!m_handle) {
		m_handle = new EventHandle();
		m_handle->isTimerCreated = false;
	}
	return m_handle;
}

void HandleOwner<IEvent, EventHandle>::_deleteHandle(EventHandle* handle)
{
	delete handle;
}

}
