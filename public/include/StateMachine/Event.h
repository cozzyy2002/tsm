#pragma once

#include "Interface.h"

namespace tsm {

class Event : public IEvent
{
public:
	Event() : m_priority(0) {}
	virtual ~Event() {}

	virtual int _getPriority() const { return m_priority; }
	virtual void _setPriority(int priority) { m_priority = priority; }

protected:
	int m_priority;
};

}
