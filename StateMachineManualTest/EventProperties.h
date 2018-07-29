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

protected:
	MyContext* context;

	CMFCPropertyGridProperty* eventNameProperty;

	CMFCPropertyGridProperty* timerTypeProperty;
	CMFCPropertyGridProperty* delayProperty;
	CMFCPropertyGridProperty* intervalProperty;

	CMFCPropertyGridProperty* callExitOnShutdownProperty;

	std::tstring getStringPropertyValue(CMFCPropertyGridProperty* property);

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

	template<typename T>
	void setOptionProperty(const OptionItem<T>* optionItems, size_t optionItemCount, CMFCPropertyGridProperty* property);
	template<typename T, int optionItemCount>
	void setOptionProperty(const OptionItem<T>(&optionItems)[optionItemCount], CMFCPropertyGridProperty* property);
	template<typename T>
	void setOptionProperty(const std::vector<OptionItem<T>>& optionItems, CMFCPropertyGridProperty* property);
	template<typename T>
	T getOptionPropertyValue(const OptionItem<T>* optionItems, size_t optionItemCount, CMFCPropertyGridProperty* property);
	template<typename T, int optionItemCount>
	T getOptionPropertyValue(const OptionItem<T>(&optionItems)[optionItemCount], CMFCPropertyGridProperty* property);
	template<typename T>
	T getOptionPropertyValue(const std::vector<OptionItem<T>>& optionItems, CMFCPropertyGridProperty* property);
};
