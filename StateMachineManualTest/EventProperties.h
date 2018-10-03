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

	std::tstring getStringPropertyValue(const CMFCPropertyGridProperty* property) const;
	bool getBoolPropertyValue(const CMFCPropertyGridProperty* property) const;

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

	static const OptionItem<TimerType> timerTypeOptions[];
	std::vector<OptionItem<MyState*>> stateList;

	template<typename T>
	void setOptionProperty(const OptionItem<T>* optionItems, size_t optionItemCount, CMFCPropertyGridProperty* property);
	template<typename T, int optionItemCount>
	void setOptionProperty(const OptionItem<T>(&optionItems)[optionItemCount], CMFCPropertyGridProperty* property);
	template<typename T>
	void setOptionProperty(const std::vector<OptionItem<T>>& optionItems, CMFCPropertyGridProperty* property);
	template<typename T>
	T getOptionPropertyValue(const OptionItem<T>* optionItems, size_t optionItemCount, const CMFCPropertyGridProperty* property, T defaultValue) const;
	template<typename T, int optionItemCount>
	T getOptionPropertyValue(const OptionItem<T>(&optionItems)[optionItemCount], const CMFCPropertyGridProperty* property, T defaultValue) const;
	template<typename T>
	T getOptionPropertyValue(const std::vector<OptionItem<T>>& optionItems, const CMFCPropertyGridProperty* property, T defaultValue) const;

	// MFCPropertyGridProperty class specialized to select HRESULT value.
	class CHResultProperty : public CMFCPropertyGridProperty
	{
	public:
		CHResultProperty(CEventProperties* owner, const CString& strName, LPCTSTR lpszDescr = NULL);
		HRESULT getSelectedValue() const;

	protected:
		static const OptionItem<HRESULT> VALUE_LIST[];
		CEventProperties* owner;
	};
};
