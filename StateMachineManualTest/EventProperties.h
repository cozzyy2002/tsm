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

	CMFCPropertyGridProperty* timerTypeProperty;
	CMFCPropertyGridProperty* delayProperty;
	CMFCPropertyGridProperty* intervalProperty;

	CMFCPropertyGridProperty* nextStateProperty;
	CList* nextStates;
	CMFCPropertyGridProperty* masterStates;
	CMFCPropertyGridProperty* callExitOnShutdownProperty;

	void updateNextStates();

	std::tstring getStringPropertyValue(CMFCPropertyGridProperty* property);
	bool getBoolPropertyValue(CMFCPropertyGridProperty* property);

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
	T getOptionPropertyValue(const OptionItem<T>* optionItems, size_t optionItemCount, CMFCPropertyGridProperty* property, T defaultValue);
	template<typename T, int optionItemCount>
	T getOptionPropertyValue(const OptionItem<T>(&optionItems)[optionItemCount], CMFCPropertyGridProperty* property, T defaultValue);
	template<typename T>
	T getOptionPropertyValue(const std::vector<OptionItem<T>>& optionItems, CMFCPropertyGridProperty* property, T defaultValue);
};
