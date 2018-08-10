#pragma once

class MyContext;
class MyEvent;
class MyState;

class CEventProperties : public CMFCPropertyGridCtrl
{
public:
	CEventProperties(MyContext* context);
	~CEventProperties();

	virtual void Init() override;
	void updateStates();
	MyEvent* createEvent();

protected:
	MyContext* context;

	CMFCPropertyGridProperty* eventNameProperty;

	CMFCPropertyGridProperty* timerTypeProperty;
	CMFCPropertyGridProperty* delayProperty;
	CMFCPropertyGridProperty* intervalProperty;

	CMFCPropertyGridProperty* nextStateProperty;
	CMFCPropertyGridProperty* nextStates;
	CMFCPropertyGridProperty* masterStates;
	CMFCPropertyGridProperty* callExitOnShutdownProperty;

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
