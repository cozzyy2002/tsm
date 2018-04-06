#include "stdafx.h"
#include "MyLogger.h"

static log4cplus::Logger logger = log4cplus::Logger::getInstance(_T("Logger"));

template<class T>
LPCSTR getClassName(T* obj)
{
	return obj ? typeid(*obj).name() : "<nullptr>";
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
	LOG4CPLUS_INFO(logger, "Handling event " << getClassName(event) << " in state " << getClassName(current));
}

void MyLogger::onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
{
	LOG4CPLUS_INFO(logger, "State changed from " << getClassName(previous) << " to " << getClassName(next));
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
	LOG4CPLUS_ERROR(logger, exp << " failed. HRESULT=0x" << std::hex << hr << ". at:\n" << sourceFile << "#L" << line);
}
