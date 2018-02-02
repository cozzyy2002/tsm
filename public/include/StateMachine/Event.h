#pragma once

#include <StateMachine/Interface.h>

namespace tsm {

class Event : public IEvent
{
public:
	Event() {}
	virtual ~Event() {}
};

}
