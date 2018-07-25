#include "stdafx.h"
#include "EventProperties.h"
#include "MyContext.h"
#include "MyEvent.h"

CEventProperties::CEventProperties()
	: context(nullptr), ctrl(nullptr)
{
}


CEventProperties::~CEventProperties()
{
}

template<typename T>
struct OptionItem {
	LPCTSTR name;
	T value;
};

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


static const OptionItem<CEventProperties::TimerType> timerTypeOptions[] = {
	{ _T("None"), CEventProperties::TimerType::None },
	{ _T("Context"), CEventProperties::TimerType::Context },
	{ _T("State"), CEventProperties::TimerType::State },
};

void CEventProperties::initialize(MyContext * context, CMFCPropertyGridCtrl * ctrl)
{
	this->context = context;
	this->ctrl = ctrl;

	// Event name: string
	eventNameProperty = new CMFCPropertyGridProperty(_T("Name"), COleVariant(_T("")), _T("Event name"));
	ctrl->AddProperty(eventNameProperty);

	// Timer property: Timer type, delay, interval
	auto timerPropertiesProperty = new CMFCPropertyGridProperty(_T("Timer"));
	timerTypeProperty = new CMFCPropertyGridProperty(_T("Type"), _T("None"), _T("Select timer type"));
	setOptionProperty(timerTypeOptions, timerTypeProperty);
	timerTypeProperty->AllowEdit(FALSE);
	timerPropertiesProperty->AddSubItem(timerTypeProperty);
	delayProperty = new CMFCPropertyGridProperty(_T("Delay"), COleVariant((long)0), _T("Delay(mSec)"));
	timerPropertiesProperty->AddSubItem(delayProperty);
	intervalProperty = new CMFCPropertyGridProperty(_T("Interval"), COleVariant((long)0), _T("Interval(mSec)"));
	timerPropertiesProperty->AddSubItem(intervalProperty);
	ctrl->AddProperty(timerPropertiesProperty);

	// Next state
	auto nextStateProperty = new CMFCPropertyGridProperty(_T("Next state"));
	VARIANT _callExitOnShutdown = { VT_BOOL };
	callExitOnShutdownProperty = new CMFCPropertyGridProperty(_T("callExitOnShutdown"), COleVariant(_callExitOnShutdown));
	nextStateProperty->AddSubItem(callExitOnShutdownProperty);
	ctrl->AddProperty(nextStateProperty);

	ctrl->SetGroupNameFullWidth();
	ctrl->RedrawWindow();
}

MyEvent * CEventProperties::createEvent()
{
	if(!context) return nullptr;

	auto e = new MyEvent(*context, eventNameProperty->GetValue().bstrVal);

	auto timerType = getOptionPropertyValue(timerTypeOptions, timerTypeProperty);
	tsm::TimerClient* timerClient;
	switch(timerType) {
	default:
	case TimerType::None:
		timerClient = nullptr;
		break;
	case TimerType::Context:
		timerClient = context;
		break;
	case TimerType::State:
		timerClient = context->getCurrentState();
		break;
	}
	if(timerClient) {
		e->setTimer(timerClient, delayProperty->GetValue().lVal, intervalProperty->GetValue().lVal);
	}
	return e;
}


template<typename T>
void setOptionProperty(const OptionItem<T>* optionItems, size_t optionItemCount, CMFCPropertyGridProperty* property)
{
	for(size_t i = 0; i < optionItemCount; i++) {
		property->AddOption(optionItems[i].name);
	}
}

template<typename T, int optionItemCount>
void setOptionProperty(const OptionItem<T> (&optionItems)[optionItemCount], CMFCPropertyGridProperty* property)
{
	setOptionProperty(optionItems, optionItemCount, property);
}

template<typename T>
void setOptionProperty(const std::vector<OptionItem<T>>& optionItems, CMFCPropertyGridProperty* property)
{
	setOptionProperty(optionItems.data(), optionItems.size(), property);
}

template<typename T>
T getOptionPropertyValue(const OptionItem<T>* optionItems, size_t optionItemCount, CMFCPropertyGridProperty* property)
{
	auto var = property->GetValue();
	std::tstring valueStr =
#ifdef _UNICODE
		var.bstrVal;
#else
		var.pcVal;
#endif
	for(size_t i = 0; i < optionItemCount; i++) {
		auto& optionItem = optionItems[i];
		if(valueStr == optionItem.name) return optionItem.value;
	}
	return optionItems[0].value;
}

template<typename T, int optionItemCount>
T getOptionPropertyValue(const OptionItem<T> (&optionItems)[optionItemCount], CMFCPropertyGridProperty* property)
{
	return getOptionPropertyValue(optionItems, optionItemCount, property);
}

template<typename T>
T getOptionPropertyValue(const std::vector<OptionItem<T>>& optionItems, CMFCPropertyGridProperty* property)
{
	return getOptionPropertyValue(optionItems.data(), optionItems.size(), property);
}
