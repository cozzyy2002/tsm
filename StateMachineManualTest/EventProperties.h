#pragma once

#include <functional>

class MyContext;
class MyEvent;
class MyState;

class CEventProperties : public CMFCPropertyGridCtrl
{
public:
	CEventProperties(MyContext* context);
	~CEventProperties();

	virtual void Init() override;
	virtual void AdjustLayout() override;
	void updateStates();
	MyEvent* createEvent();

	enum class TimerType {
		None,
		Context,
		State,
	};

	template<typename T>
	struct OptionItem {
		LPCTSTR name;
		T value;
	};

protected:
	MyContext* context;

	class CList : public CMFCPropertyGridProperty
	{
	public:
		CList(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
			LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL, LPCTSTR lpszValidChars = NULL)
			: CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars) {}

		// Function called when combobox value is changed such as:
		//   Text is edited
		//   Item selected is changed
		std::function<void()> onValueUpdated;

	protected:
		virtual BOOL OnUpdateValue() override;
	};

	CMFCPropertyGridProperty* eventNameProperty;
	CMFCPropertyGridProperty* eventPriorityProperty;

	CMFCPropertyGridProperty* timerTypeProperty;
	CMFCPropertyGridProperty* delayProperty;
	CMFCPropertyGridProperty* intervalProperty;

	CMFCPropertyGridProperty* nextStateProperty;
	CList* nextStates;
	CMFCPropertyGridProperty* masterStates;
	CMFCPropertyGridProperty* callExitOnShutdownProperty;

	class CHResultProperty;
	CHResultProperty* handleEventHResultProperty;
	CHResultProperty* entryHResultProperty;
	CHResultProperty* exitHResultProperty;

	void updateNextStates();

	static const OptionItem<TimerType> timerTypeOptions[];
	std::vector<OptionItem<MyState*>> stateList;

	// MFCPropertyGridProperty class specialized to select HRESULT value.
	class CHResultProperty : public CMFCPropertyGridProperty
	{
	public:
		CHResultProperty(const CString& strName, LPCTSTR lpszDescr = NULL);
		HRESULT getSelectedValue() const;

	protected:
		static const OptionItem<HRESULT> VALUE_LIST[];
	};
};
