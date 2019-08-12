#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>

template<class C>
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

class MockContext : public tsm::Context<MockEvent<MockContext>, MockState<MockContext>>, public TestStateMonitor
{
public:
	virtual IStateMonitor* _getStateMonitor() override { return this; }
};

class MockAsyncContext : public tsm::AsyncContext<MockEvent<MockAsyncContext>, MockState<MockAsyncContext>>, public TestStateMonitor
{
public:
	virtual IStateMonitor* _getStateMonitor() override { return this; }
	HRESULT waitReady(DWORD timeout = 100);
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

template<class C>
class MockEvent : public tsm::Event<C>, public TestUnknown
{
public:
	MockEvent(int id = 0) : TestUnknown(m_cRef), id(id) { setObject(this); }

	MOCK_METHOD1_T(preHandle, HRESULT(C*));
	MOCK_METHOD2_T(postHandle, HRESULT(C*, HRESULT));

	void setPriority(int priority) { m_priority = priority; }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return TestUnknown::Release(); }

	int id;
};

template<class C>
class MockState : public tsm::State<C, MockEvent<C>>, public TestUnknown
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

	MOCK_METHOD3_T(handleEvent, HRESULT(C*, MockEvent<C>*, IState**));
	MOCK_METHOD3_T(entry, HRESULT(C*, MockEvent<C>*, IState*));
	MOCK_METHOD3_T(exit, HRESULT(C*, MockEvent<C>*, IState*));
};
