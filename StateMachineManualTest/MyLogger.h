#pragma once
#include <StateMachine/Interface.h>

class MyLogger : public tsm::IStateMonitor
{
public:
	virtual void onIdle(tsm::IContext* context) override;
	virtual void onEventTriggered(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current) override;
	virtual void onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next) override;
	virtual void onTimerStarted(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode) override;

	static void onAssertFailed(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);

	template<class T>
	static LPCTSTR toString(T* obj);
};
