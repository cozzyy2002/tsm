#pragma once

#include <StateMachine/Context.h>
#include "MyObject.h"
#include "ReportView.h"

class MyState;
class MyEvent;

class MyContext : public tsm::AsyncContext<MyEvent, MyState>, tsm::IStateMonitor, public MyObject, public ILogger
{
public:
	MyContext();

	virtual tsm::IStateMonitor* _getStateMonitor() override { return this; }
	void createStateMachine(HWND hWnd, UINT msg);
	MyState* findState(const std::tstring& name) const;

	void setLogWindow(HINSTANCE hInst, HWND hWndLog, UINT logMsg);
	virtual void log(LPCTSTR fmt, ...) override;
	LRESULT onLogMsg(HWND hWnd, WPARAM wParam, LPARAM lParam);
	CReportView* getLogView() { return &m_logView; }
	void setStatesView(HWND hWndStates);
	MyState* getSelectedState() const;

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

	// Window handle to which log message is written.
	HWND m_hWndLog;

	UINT m_logMsg;

	int m_logNo;
	DWORD m_startTime;
	CReportView m_logView;
	CReportView m_statesView;
	static const CReportView::Column m_logColumns[];
	static const CReportView::Column m_statesColumns[];
};
