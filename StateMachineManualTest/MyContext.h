#pragma once

#include <StateMachine/Context.h>
#include "MyObject.h"

class MyState;
class MyEvent;

class MyContext : public tsm::AsyncContext<MyEvent, MyState>, tsm::IStateMonitor, public MyObject
{
public:
	MyContext();

	virtual tsm::IStateMonitor* _getStateMonitor() override { return this; }
	void createStateMachine(HWND hWnd, UINT msg);
	MyState* findState(const std::tstring& name) const;

#pragma region Implementation of IStateMonitor
	virtual void onIdle(tsm::IContext* context) override;
	virtual void onEventTriggered(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onEventHandling(tsm::IContext* context, tsm::IEvent* event, tsm::IState* current) override;
	virtual void onStateChanged(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previous, tsm::IState* next) override;
	virtual void onTimerStarted(tsm::IContext* context, tsm::IEvent* event) override;
	virtual void onWorkerThreadExit(tsm::IContext* context, HRESULT exitCode) override;
#pragma endregion

	static void onAssertFailed(HRESULT hr, LPCTSTR exp, LPCTSTR sourceFile, int line);

	template<class T>
	static LPCTSTR toString(T* obj);

protected:
	// Window handle to process window messages.
	HWND m_hWnd;
};
