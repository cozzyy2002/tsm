#pragma once

#include <StateMachine/Interface.h>

#include <memory>
namespace tsm {

template<class E, class S>
class Context : public IContext
{
public:
	Context() : m_stateMachine(createStateMachine()) {}
	virtual ~Context() {}

	HRESULT setup(S* initialState, E* event = nullptr) { return m_stateMachine->setup(this, initialState, event); }
	HRESULT shutdown() { return m_stateMachine->shutdown(this); }
	HRESULT handleEvent(E* event) { return m_stateMachine->handleEvent(this, event); }

	S* getCurrentState() const { return (S*)m_currentState.p; }

protected:
	std::unique_ptr<IStateMachine> m_stateMachine;
};

}
