#pragma once

#include "Interface.h"

namespace tsm {

template<class C = IContext, class E = IEvent>
class State : public IState
{
public:
	State(IState* masterState = nullptr) : m_masterState(masterState) {}
	virtual ~State() {}

#pragma region Implementation of IState that call methods of sub class.
	HRESULT _handleEvent(IContext* context, IEvent* event, IState** nextState) {
		return handleEvent((C*)context, (E*)event, nextState);
	}
	HRESULT _entry(IContext* context, IEvent* event, IState* previousState) {
		return entry((C*)context, (E*)event, previousState);
	}
	HRESULT _exit(IContext* context, IEvent* event, IState* nextState) {
		return exit((C*)context, (E*)event, nextState);
	}

	virtual IState* _getMasterState() { return m_masterState; }
	virtual void _setMasterState(IState* state) { m_masterState = state; }
	virtual bool _hasEntryCalled() { return m_entryCalled; }
	virtual void _setEntryCalled(bool value) { m_entryCalled = value; }
#pragma endregion

#pragma region Methods to be implemented by sub class.
	virtual HRESULT handleEvent(C*, E* event, IState** nextState) { return S_FALSE; }
	virtual HRESULT entry(C* context, E* event, IState* previousState) { return S_FALSE; }
	virtual HRESULT exit(C* context, E* event, IState* nextState) { return S_FALSE; }
#pragma endregion

	State* getMasterState() { return (State*)m_masterState; }
	State* getSubState() { return (State*)m_subState.p; }

	bool isSubState() const { return m_masterState ? true : false; }

protected:
	CComPtr<IState> m_masterState;
	bool m_entryCalled;
};

}
