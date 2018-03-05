#pragma once

#include "Interface.h"

namespace tsm {

class Event : public IEvent
{
public:
	// Default event priority.
	// This value is used by AsyncContext::triggerEvent().
	// Context::triggerEvent() is not implemented.
	static const int DefaultPriority = 0;

	Event(int priority = DefaultPriority) : m_priority(priority) {}
	virtual ~Event() {}

	virtual int _getPriority() const override { return m_priority; }

protected:
	int m_priority;
};

}
