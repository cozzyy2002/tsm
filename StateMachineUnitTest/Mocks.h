#pragma once

#include <StateMachine/Context.h>
#include <StateMachine/Event.h>
#include <StateMachine/State.h>

class MockEvent;
template <class C>
class MockState;

class TestStateMonitor : public tsm::IStateMonitor
{
public:
#define PTR2STR(p) ((p) ? typeid(*p).name() : "<nullptr>")

	virtual void onIdle(tsm::IContext* context, bool setupCompleted) {
		printf_s(__FUNCTION__ ": %s(0x%p)%s\n", PTR2STR(context), context, setupCompleted ? " - Setup completed" : "");
	}
	virtual void onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next) {
		printf_s(__FUNCTION__ ": %s(0x%p): IEvent=%s(0x%p), %s(0x%p)->%s(0x%p)\n",
			PTR2STR(context), context, PTR2STR(event), event, PTR2STR(previous), previous, PTR2STR(next), next);
	}
	virtual void onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode) {
		printf_s(__FUNCTION__ ": %s(0x%p): HRESULT=0x%08x\n", PTR2STR(context), context, exitCode);
	}
};

class MockContext : public tsm::Context<MockEvent, MockState<MockContext>>, public TestStateMonitor
{
public:
	//virtual IStateMonitor* _getStateMonitor() { return this; }
};

class MockAsyncContext : public tsm::AsyncContext<MockEvent, MockState<MockAsyncContext>>, public TestStateMonitor
{
public:
	//virtual IStateMonitor* _getStateMonitor() { return this; }
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
