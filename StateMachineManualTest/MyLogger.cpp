#include "stdafx.h"
#include "MyLogger.h"

void MyLogger::onIdle(tsm::IContext* context)
{
}

void MyLogger::onEventTriggered(tsm::IContext* context, tsm::IEvent* event)
{
}

void MyLogger::onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current)
{
}

void MyLogger::onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next)
{
}

void MyLogger::onTimerStarted(tsm::IContext* context, tsm::IEvent* event)
{
}

void MyLogger::onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode)
{
}

void MyLogger::onAssertFailed(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
}
