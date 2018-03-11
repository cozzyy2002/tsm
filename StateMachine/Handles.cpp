#include <StateMachine/stdafx.h>

#include "Handles.h"

namespace tsm {

template<>
EventHandle* HandleOwner<IEvent, EventHandle>::_getHandle()
{
	if(!m_handle) m_handle = _createHandle(_getInstance());
	return m_handle;
}

template<>
EventHandle* HandleOwner<IEvent, EventHandle>::_createHandle(IEvent*)
{
	auto handle = new EventHandle();
	handle->isTimerCreated = false;

	return handle;
}

template<>
void HandleOwner<IEvent, EventHandle>::_deleteHandle(EventHandle* handle)
{
	delete handle;
}

template<>
ContextHandle* HandleOwner<IContext, ContextHandle>::_getHandle()
{
	if(!m_handle) m_handle = _createHandle(_getInstance());
	return m_handle;
}

template<>
ContextHandle* HandleOwner<IContext, ContextHandle>::_createHandle(IContext* context)
{
	auto handle = new ContextHandle();
	if(context->isAsync()) {
		handle->_asyncData.reset(new ContextHandle::AsyncData());
	}
	handle->asyncData = handle->_asyncData.get();

	return handle;
}

template<>
void HandleOwner<IContext, ContextHandle>::_deleteHandle(ContextHandle* handle)
{
	delete handle;
}

}
