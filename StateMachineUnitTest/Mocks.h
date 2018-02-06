#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>

class MockEvent;
class MockState;
class MockAsyncState;

class MockContext : public tsm::Context<MockEvent, MockState>
{
public:
};

class MockAsyncContext : public tsm::AsyncContext<MockEvent, MockAsyncState>
{
public:
};

/* Base class for Mock class inheriting IUnknown */
class TestUnknown
{
public:
	TestUnknown(ULONG& cRef) : rcRef(cRef), isReleaseCalled(false) {}
	virtual ~TestUnknown();

	ULONG getReferenceCount() const { return rcRef; }
	bool deleted() const;
	ULONG Release();

protected:
	// Initialize className using _this pointer.
	void setObject(IUnknown* _this);

	ULONG& rcRef;
	bool isReleaseCalled;
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

class MockAsyncState : public tsm::State<MockAsyncContext, MockEvent, MockAsyncState>, public TestUnknown
{
public:
	MockAsyncState() : TestUnknown(m_cRef) { setObject(this); }

	void setMasterState(MockState* masterState) { m_masterState = masterState; }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return TestUnknown::Release(); }

	MOCK_METHOD3(handleEvent, HRESULT(MockAsyncContext*, MockEvent*, MockAsyncState**));
	MOCK_METHOD3(entry, HRESULT(MockAsyncContext*, MockEvent*, MockAsyncState*));
	MOCK_METHOD3(exit, HRESULT(MockAsyncContext*, MockEvent*, MockAsyncState*));
};
