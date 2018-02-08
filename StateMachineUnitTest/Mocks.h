#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>

class MockEvent;
template <class C>
class MockState;

class MockContext : public tsm::Context<MockEvent, MockState<MockContext>> {};
class MockAsyncContext : public tsm::AsyncContext<MockEvent, MockState<MockAsyncContext>> {};

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

template<class C>
class MockState : public tsm::State<C, MockEvent>, public TestUnknown
{
public:
	MockState() : TestUnknown(m_cRef) { setObject(this); }
	virtual ~MockState() {}

	void setMasterState(MockState* masterState) { m_masterState = masterState; }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return TestUnknown::Release(); }

	MOCK_METHOD3_T(handleEvent, HRESULT(C*, MockEvent*, IState**));
	MOCK_METHOD3_T(entry, HRESULT(C*, MockEvent*, IState*));
	MOCK_METHOD3_T(exit, HRESULT(C*, MockEvent*, IState*));
};
