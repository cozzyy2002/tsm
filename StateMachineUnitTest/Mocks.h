#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>

class MockEvent;
template <class C>
class MockState;

template<class T>
log4cplus::tstring ptr2str(T* ptr)
{
	if(ptr) {
		CA2T str(typeid(*ptr).name());
		return (LPCTSTR)str;
	} else {
		return _T("<nullptr>");
	}
}

class TestStateMonitor : public tsm::IStateMonitor
{
public:
	virtual void onIdle(tsm::IContext* context) override;
	virtual void onEventTriggered(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current) override;
	virtual void onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next) override;
	virtual void onTimerStarted(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode) override;
};

extern void mockOnAssertFailed(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);

class MockContext : public tsm::Context<MockEvent, MockState<MockContext>>, public TestStateMonitor
{
public:
	virtual IStateMonitor* _getStateMonitor() override { return this; }
};

class MockAsyncContext : public tsm::AsyncContext<MockEvent, MockState<MockAsyncContext>>, public TestStateMonitor
{
public:
	virtual IStateMonitor* _getStateMonitor() override { return this; }
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
	MockEvent(int id = 0) : TestUnknown(m_cRef), id(id) { setObject(this); }

	void setPriority(int priority) { m_priority = priority; }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return TestUnknown::Release(); }

	int id;
};

template<class C>
class MockState : public tsm::State<C, MockEvent>, public TestUnknown
{
public:
	MockState() : TestUnknown(m_cRef) { setObject(this); }
	virtual ~MockState() {}

	void setMasterState(MockState* masterState) { m_masterState = masterState; }
	virtual ULONG STDMETHODCALLTYPE Release(void) {
		auto cRef = TestUnknown::Release();
		// Do as if deleted.
		if(cRef == 0) m_masterState.Release();
		return cRef;
	}

	MOCK_METHOD3_T(handleEvent, HRESULT(C*, MockEvent*, IState**));
	MOCK_METHOD3_T(entry, HRESULT(C*, MockEvent*, IState*));
	MOCK_METHOD3_T(exit, HRESULT(C*, MockEvent*, IState*));
};
