#include "stdafx.h"
#include "EventProperties.h"
#include "MyContext.h"
#include "MyEvent.h"
#include "MyState.h"

CEventProperties::CEventProperties(MyContext* context)
	: context(context)
	, nextStates(nullptr), masterStates(nullptr)
{
}


CEventProperties::~CEventProperties()
{
}


/*static*/ const CEventProperties::OptionItem<CEventProperties::TimerType> CEventProperties::timerTypeOptions[] = {
	{ _T("None"), CEventProperties::TimerType::None },
	{ _T("Context"), CEventProperties::TimerType::Context },
	{ _T("State"), CEventProperties::TimerType::State },
};

void CEventProperties::Init()
{
	CMFCPropertyGridCtrl::Init();

	// Event name: string
	eventNameProperty = new CMFCPropertyGridProperty(_T("Name"), COleVariant(_T("")), _T("Event name"));
	AddProperty(eventNameProperty);

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
	AddProperty(timerPropertiesProperty);

	// Next state
	nextStateProperty = new CMFCPropertyGridProperty(_T("Next state"));
	VARIANT _callExitOnShutdown = { VT_BOOL };
	callExitOnShutdownProperty = new CMFCPropertyGridProperty(_T("callExitOnShutdown"), COleVariant(_callExitOnShutdown));
	nextStateProperty->AddSubItem(callExitOnShutdownProperty);
	AddProperty(nextStateProperty);

	RedrawWindow();
}

static MyState* NewNextState = (MyState*)-1;

void CEventProperties::updateStates()
{
	if(nextStates) {
		nextStateProperty->RemoveSubItem(nextStates, FALSE);
		nextStates->RemoveAllOptions();
		nextStateProperty->RemoveSubItem(masterStates, FALSE);
		masterStates->RemoveAllOptions();
	} else {
		// Next state property contains empty item and state list excluding current state.
		// Empty item is editable to specify name of new state.
		nextStates = new CMFCPropertyGridProperty(_T("Name"), _T(""), _T("Type new state name or select existing state"));
		nextStates->AllowEdit(TRUE);
		// Master state property containg empty item and state list including current state.
		// Empty item is non-editable and selecting it means that next state does nat have master state.
		masterStates = new CMFCPropertyGridProperty(_T("Master state"), _T(""), _T("Select existing state"));
		masterStates->AllowEdit(FALSE);
	}
	nextStates->AddOption(_T(""));
	masterStates->AddOption(_T(""));
	nextStateProperty->RemoveSubItem(callExitOnShutdownProperty, FALSE);

	stateList.clear();
	for(auto state = context->getCurrentState(); state; state = state->getMasterState()) {
		OptionItem<MyState*> item = { state->getName().c_str(), state };
		stateList.push_back(item);
	}

	if(1 < stateList.size()) {
		setOptionProperty(&(stateList.data()[1]), stateList.size() - 1, nextStates);
	}
	setOptionProperty(stateList, masterStates);

	nextStateProperty->AddSubItem(nextStates);
	nextStateProperty->AddSubItem(masterStates);
	nextStateProperty->AddSubItem(callExitOnShutdownProperty);

	RedrawWindow();
}

MyEvent * CEventProperties::createEvent()
{
	// Create event.
	auto e = new MyEvent(*context, getStringPropertyValue(eventNameProperty));

	// Set timer to the event.
	auto timerType = getOptionPropertyValue(timerTypeOptions, timerTypeProperty, TimerType::None);
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

	// Get next state and master state of the state.
	auto nextState = getOptionPropertyValue(&(stateList.data()[1]), stateList.size() - 1, nextStates, (MyState*)nullptr);
	if(!nextState) {
		auto nextStateName = getStringPropertyValue(nextStates);
		if(!nextStateName.empty()) {
			// Create new state and set it as next state.
			auto masterState = getOptionPropertyValue(stateList, masterStates, (MyState*)nullptr);
			e->nextState = new MyState(*context, nextStateName, masterState);
			e->nextState.p->callExitOnShutDown = getBoolPropertyValue(callExitOnShutdownProperty);
		}
	}

	return e;
}

std::tstring CEventProperties::getStringPropertyValue(CMFCPropertyGridProperty * property)
{
	auto var = property->GetValue();
	return
#ifdef _UNICODE
		var.bstrVal;
#else
		var.pcVal;
#endif
}

bool CEventProperties::getBoolPropertyValue(CMFCPropertyGridProperty * property)
{
	auto var = property->GetValue();
	return var.boolVal ? true : false;
}

template<typename T>
void CEventProperties::setOptionProperty(const OptionItem<T>* optionItems, size_t optionItemCount, CMFCPropertyGridProperty* property)
{
	//if(0 < optionItemCount) {
	//	// Select first item.
	//	COleVariant var(optionItems[0].name);
	//	property->SetValue(var);
	//}

	for(size_t i = 0; i < optionItemCount; i++) {
		property->AddOption(optionItems[i].name);
	}
}

template<typename T, int optionItemCount>
void CEventProperties::setOptionProperty(const OptionItem<T> (&optionItems)[optionItemCount], CMFCPropertyGridProperty* property)
{
	setOptionProperty(optionItems, optionItemCount, property);
}

template<typename T>
void CEventProperties::setOptionProperty(const std::vector<OptionItem<T>>& optionItems, CMFCPropertyGridProperty* property)
{
	setOptionProperty(optionItems.data(), optionItems.size(), property);
}

template<typename T>
T CEventProperties::getOptionPropertyValue(const OptionItem<T>* optionItems, size_t optionItemCount, CMFCPropertyGridProperty* property, T defaultValue)
{
	auto str = getStringPropertyValue(property);
	for(size_t i = 0; i < optionItemCount; i++) {
		auto& optionItem = optionItems[i];
		if(str == optionItem.name) return optionItem.value;
	}
	return defaultValue;
}

template<typename T, int optionItemCount>
T CEventProperties::getOptionPropertyValue(const OptionItem<T> (&optionItems)[optionItemCount], CMFCPropertyGridProperty* property, T defaultValue)
{
	return getOptionPropertyValue(optionItems, optionItemCount, property, defaultValue);
}

template<typename T>
T CEventProperties::getOptionPropertyValue(const std::vector<OptionItem<T>>& optionItems, CMFCPropertyGridProperty* property, T defaultValue)
{
	return getOptionPropertyValue(optionItems.data(), optionItems.size(), property, defaultValue);
}
