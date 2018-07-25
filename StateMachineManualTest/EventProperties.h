#pragma once

class MyContext;
class MyEvent;

class CEventProperties
{
public:
	CEventProperties();
	~CEventProperties();

	void initialize(MyContext* context, CMFCPropertyGridCtrl* ctrl);
	MyEvent* createEvent();

	enum class TimerType {
		None,
		Context,
		State,
	};

protected:
	MyContext* context;
	CMFCPropertyGridCtrl* ctrl;

	CMFCPropertyGridProperty* eventNameProperty;

	CMFCPropertyGridProperty* timerTypeProperty;
	CMFCPropertyGridProperty* delayProperty;
	CMFCPropertyGridProperty* intervalProperty;

	CMFCPropertyGridProperty* callExitOnShutdownProperty;
};
