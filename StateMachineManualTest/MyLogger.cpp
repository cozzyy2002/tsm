#include "stdafx.h"

#include "MyObject.h"
#include "MyLogger.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("Logger"));

template<class T>
LPCTSTR MyLogger::toString(T* obj)
{
	auto _obj = dynamic_cast<MyObject*>(obj);
	return _obj ? _obj->toString() : _T("<nullptr>");
}

void MyLogger::onIdle(tsm::IContext* context)
{
	LOG4CPLUS_INFO(logger, "StateMachined is idle.");
}

void MyLogger::onEventTriggered(tsm::IContext* context, tsm::IEvent* event)
{
}

void MyLogger::onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
{
	LOG4CPLUS_INFO(logger, "Handling event " << toString(event) << " in state " << toString(current));
}

void MyLogger::onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
{
	LOG4CPLUS_INFO(logger, "State changed from " << toString(previous) << " to " << toString(next));
}

void MyLogger::onTimerStarted(tsm::IContext* context, tsm::IEvent* event)
{
}

void MyLogger::onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode)
{
	LOG4CPLUS_INFO(logger, "Worker thread exit. Code=0x" << std::hex << exitCode);
}

void MyLogger::onAssertFailed(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	LOG4CPLUS_ERROR(logger, exp << " failed. HRESULT=0x" << std::hex << hr << ". at:\n" << sourceFile << "#L" << std::dec << line);
}
