#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>

class MockEvent;
class MockState;

class MockContext : public tsm::Context<MockEvent, MockState>
{
public:
};

/* Base class for Mock class inheriting IUnknown */
class TestUnknown
{
public:
	TestUnknown(ULONG& cRef) : rcRef(cRef) {}
	virtual ~TestUnknown();

	bool deleted() const;
	ULONG Release();

protected:
	// Initialize className using _this pointer.
	void setObject(IUnknown* _this);

	ULONG& rcRef;
	std::string className;
};

class MockEvent : public tsm::Event, public TestUnknown
{
public:
	MockEvent() : TestUnknown(m_cRef) { setObject(this); }

	virtual ULONG STDMETHODCALLTYPE Release(void) { return TestUnknown::Release(); }
};

class MockState : public tsm::State<MockContext, MockEvent, MockState>, public TestUnknown
{
public:
	MockState() : TestUnknown(m_cRef) { setObject(this); }

	void setMasterState(MockState* masterState) { m_masterState = masterState; }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return TestUnknown::Release(); }

	MOCK_METHOD3(handleEvent, HRESULT(MockContext*, MockEvent*, MockState**));
	MOCK_METHOD3(entry, HRESULT(MockContext*, MockEvent*, MockState*));
	MOCK_METHOD3(exit, HRESULT(MockContext*, MockEvent*, MockState*));
};
