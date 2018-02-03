#pragma once

#include "Interface.h"

namespace tsm {

template<class C, class E, class S>
class State : public IState
{
public:
	State(IState* masterState = nullptr)
		: IState(masterState) {}
	virtual ~State() {}

#pragma region Implementation of IState that call methods of sub class.
	HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) {
		return handleEvent((C*)context, (E*)event, (S**)nextState);
	}
	HRESULT _entry(IContext* context, IEvent* event, IState* previousState) {
		return entry((C*)context, (E*)event, (S*)previousState);
	}
	HRESULT _exit(IContext* context, IEvent* event, IState* nextState) {
		return exit((C*)context, (E*)event, (S*)nextState);
	}
#pragma endregion

#pragma region Methods to be implemented by sub class.
	virtual HRESULT handleEvent(C*, E* event, S** nextState) { return S_FALSE; }
	virtual HRESULT entry(C* context, E* event, S* previousState) { return S_FALSE; }
	virtual HRESULT exit(C* context, E* event, S* nextState) { return S_FALSE; }
#pragma endregion

	S* getMasterState() { return (S*)m_masterState; }
	S* getSubState() { return (S*)m_subState.p; }

	bool isSubState() const { return m_masterState ? true : false; }
};

}
