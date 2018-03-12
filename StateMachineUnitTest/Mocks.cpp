#include "stdafx.h"

#include "Mocks.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("UnitTest.Mocks"));

void TestStateMonitor::onIdle(tsm::IContext* context) {
	LOG4CPLUS_INFO_FMT(logger, _T(__FUNCTION__) _T(": %s(0x%p)"),
		ptr2str(context).c_str(), context);
}
void TestStateMonitor::onEventTriggered(tsm::IContext * context, tsm::IEvent * event)
{
	LOG4CPLUS_INFO_FMT(logger, _T(__FUNCTION__) _T(": %s(0x%p): IEvent=%s(0x%p)"),
		ptr2str(context).c_str(), context,
		ptr2str(event).c_str(), event);
}

void TestStateMonitor::onEventHandling(tsm::IContext * context, tsm::IEvent * event, tsm::IState * current)
{
	LOG4CPLUS_INFO_FMT(logger, _T(__FUNCTION__) _T(": %s(0x%p): IEvent=%s(0x%p), %s(0x%p)"),
		ptr2str(context).c_str(), context,
		ptr2str(event).c_str(), event,
		ptr2str(current).c_str(), current);
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
	static const LPCTSTR format = _T("'%s' failed: HRESULT=0x%08x. at:\n%s:%d");
	switch(hr) {
	case E_ABORT:
	case E_UNEXPECTED:
		LOG4CPLUS_FATAL_FMT(logger, format, exp, hr, sourceFile, line);
		break;
	default:
		LOG4CPLUS_ERROR_FMT(logger, format, exp, hr, sourceFile, line);
		break;
	}
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
