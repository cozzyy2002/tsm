#pragma once

#include <StateMachine/Interface.h>

namespace tsm {

template<class S>
class Context : public IContext
{
public:
	virtual ~Context() {}

	S* getCurrentState() const { return (S*)m_currentState.p; }
};

}
