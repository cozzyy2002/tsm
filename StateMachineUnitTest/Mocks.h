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

/* Base class for Mock class inheriting IUnknown */
class TestUnknown
{
public:
	TestUnknown(ULONG& cRef) : rcRef(cRef) {}

	bool deleted() const;
	ULONG Release();

protected:
	ULONG& rcRef;
};

class MockEvent : public tsm::Event, public TestUnknown
{
public:
	MockEvent() : TestUnknown(m_cRef) {}

	virtual ULONG STDMETHODCALLTYPE Release(void) { return TestUnknown::Release(); }
};

class MockState : public tsm::State<MockContext, MockEvent, MockState>, public TestUnknown
{
public:
	MockState() : TestUnknown(m_cRef) {}

	virtual ULONG STDMETHODCALLTYPE Release(void) { return TestUnknown::Release(); }

	MOCK_METHOD3(handleEvent, HRESULT(MockContext*, MockEvent*, MockState**));
	MOCK_METHOD3(entry, HRESULT(MockContext*, MockEvent*, MockState*));
	MOCK_METHOD3(exit, HRESULT(MockContext*, MockEvent*, MockState*));
};
