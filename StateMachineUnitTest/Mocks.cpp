#include "stdafx.h"

#include "Mocks.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("UnitTest.Mocks"));

void TestStateMonitor::onIdle(tsm::IContext* context, bool setupCompleted) {
	LOG4CPLUS_INFO_FMT(logger, _T(__FUNCTION__) _T(": %s(0x%p)%s"),
		ptr2str(context).c_str(), context, setupCompleted ? _T(" - Setup completed") : _T(""));
}
void TestStateMonitor::onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next) {
	LOG4CPLUS_INFO_FMT(logger, _T(__FUNCTION__) _T(": %s(0x%p): IEvent=%s(0x%p), %s(0x%p)->%s(0x%p)"),
		ptr2str(context).c_str(), context,
		ptr2str(event).c_str(), event,
		ptr2str(previous).c_str(), previous, ptr2str(next).c_str(), next);
}
void TestStateMonitor::onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode) {
	LOG4CPLUS_INFO_FMT(logger, _T(__FUNCTION__) _T(": %s(0x%p): HRESULT=0x%08x"),
		ptr2str(context).c_str(), context, exitCode);
}

void mockOnAssertFailed(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	LOG4CPLUS_ERROR_FMT(logger, _T("'%s' failed: HRESULT=0x%08x. at:\n%s:%d"), exp, hr, sourceFile, line);
}

TestUnknown::~TestUnknown()
{
	if(rcRef != 0) {
		ADD_FAILURE() << className << ": Reference count is not zero at destruction: " << (LONG)rcRef;
	}
}

bool TestUnknown::deleted() const
{
	if(!isReleaseCalled) ADD_FAILURE() << className << "::Release has not been called.";
	return isReleaseCalled && (rcRef == 0);
}

/*
 * Implementation of IUnknown::Release()
 *
 * This method does not delete this object even if reference count reaches zero to:
 *   Be created in stack.
 *   check whether the object is deleted by calling deleted() method.
 */
ULONG TestUnknown::Release()
{
	isReleaseCalled = true;
	LONG cRef = InterlockedDecrement(&rcRef);
	if(cRef < 0) {
		ADD_FAILURE() << className << ": Invalid reference count: " << cRef;
	}
	return rcRef;
}

void TestUnknown::setObject(IUnknown * _this)
{
	className = typeid(*_this).name();
}

HRESULT MockContext::waitReady(DWORD timeout /*= 100*/)
{
	auto hr = tsm::Context<MockEvent, MockState<MockContext>>::waitReady(timeout);
	// Wait for TestStateMonitor to log message
	Sleep(100);
	return hr;
}

HRESULT MockAsyncContext::waitReady(DWORD timeout /*= 100*/)
{
	auto hr = tsm::AsyncContext<MockEvent, MockState<MockAsyncContext>>::waitReady(timeout);
	// Wait for TestStateMonitor to log message
	Sleep(100);
	return hr;
}
