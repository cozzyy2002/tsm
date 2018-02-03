#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>

class MockContext;
class MockEvent;
class MockState;

class MockContext : public tsm::Context<MockState>
{
public:
};

class MockEvent : public tsm::Event
{
public:
	virtual ULONG STDMETHODCALLTYPE Release(void);
	ULONG getRefCount() const { return m_cRef; }
};

class MockState : public tsm::State<MockContext, MockEvent, MockState>
{
public:
	MockState() { m_cRef++; }
	virtual ULONG STDMETHODCALLTYPE Release(void);
	ULONG getRefCount() const { return m_cRef; }

	MOCK_METHOD3(handleEvent, HRESULT(MockContext*, MockEvent*, MockState**));
	MOCK_METHOD3(entry, HRESULT(MockContext*, MockEvent*, MockState*));
	MOCK_METHOD3(exit, HRESULT(MockContext*, MockEvent*, MockState*));
};
