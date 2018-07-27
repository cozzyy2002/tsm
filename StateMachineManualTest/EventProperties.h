#pragma once

class MyContext;
class MyEvent;

class CEventProperties : public CMFCPropertyGridCtrl
{
public:
	CEventProperties();
	~CEventProperties();

	void initialize(MyContext* context);
	void updateStates();
	MyEvent* createEvent();

	enum class TimerType {
		None,
		Context,
		State,
	};

protected:
	MyContext* context;

	CMFCPropertyGridProperty* eventNameProperty;

	CMFCPropertyGridProperty* timerTypeProperty;
	CMFCPropertyGridProperty* delayProperty;
	CMFCPropertyGridProperty* intervalProperty;

	CMFCPropertyGridProperty* callExitOnShutdownProperty;
};
