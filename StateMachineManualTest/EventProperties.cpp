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
	{ _T("None"), TimerType::None },
	{ _T("Context"), TimerType::Context },
	{ _T("State"), TimerType::State },
};

void CEventProperties::Init()
{
	CMFCPropertyGridCtrl::Init();

	// Event name: string
	eventNameProperty = new CMFCPropertyGridProperty(_T("Name"), COleVariant(_T("")), _T("Event name"));
	AddProperty(eventNameProperty);

	// Event priority
	eventPriorityProperty = new CMFCPropertyGridProperty(_T("Priority"), COleVariant((long)0, VT_I4), _T("Event priority"));
	eventPriorityProperty->EnableSpinControl(TRUE, -100, 100);
	AddProperty(eventPriorityProperty);

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
	updateStates();

	// HRESULT of event handlers
	auto hResultProperty = new CMFCPropertyGridProperty(_T("Return value"));
	handleEventHResultProperty = new CHResultProperty(this, _T("handleEvent()"), _T("Return value of State::handleEvent() method."));
	hResultProperty->AddSubItem(handleEventHResultProperty);
	entryHResultProperty = new CHResultProperty(this, _T("entry()"), _T("Return value of State::entry() method."));
	hResultProperty->AddSubItem(entryHResultProperty);
	exitHResultProperty = new CHResultProperty(this, _T("exit()"), _T("Return value of State::exit() method."));
	hResultProperty->AddSubItem(exitHResultProperty);
	AddProperty(hResultProperty);

	RedrawWindow();
}

void CEventProperties::AdjustLayout()
{
	CMFCPropertyGridCtrl::AdjustLayout();

	// Adjust header width.
	auto& header = GetHeaderCtrl();
	HDITEM headerItem = { HDI_WIDTH };
	header.GetItem(0, &headerItem);
	auto width = headerItem.cxy;
	header.GetItem(1, &headerItem);
	width = headerItem.cxy;
	headerItem.cxy = width / 2;
	header.SetItem(0, &headerItem);
	header.SetItem(1, &headerItem);
}

static MyState* NewNextState = (MyState*)-1;

void CEventProperties::updateStates()
{
	if(nextStates) {
		CMFCPropertyGridProperty* p = nextStates;
		nextStateProperty->RemoveSubItem(p, FALSE);
		nextStates->RemoveAllOptions();
		nextStateProperty->RemoveSubItem(masterStates, FALSE);
		masterStates->RemoveAllOptions();
	} else {
		// Next state property contains empty item and state list excluding current state.
		// Empty item is editable to specify name of new state.
		nextStates = new CList(_T("Name"), _T(""), _T("Type new state name or select existing state"));
		nextStates->AllowEdit(TRUE);
		nextStates->onValueUpdated = [this]() { updateNextStates(); };
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
	updateNextStates();
}

MyEvent * CEventProperties::createEvent()
{
	// Create event.
	auto e = new MyEvent(*context, getStringPropertyValue(eventNameProperty), eventPriorityProperty->GetValue().lVal);
	e->hrHandleEvent = handleEventHResultProperty->getSelectedValue();
	e->hrEntry = entryHResultProperty->getSelectedValue();
	e->hrExit = exitHResultProperty->getSelectedValue();

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
	// Next state may be:
	//  One of states in stateList list box excluding top state(current state)
	//  New state whose name is specified by text in nextState combo box.
	//  Or nullptr that is state change does not occur.
	MyState* nextState = nullptr;
	if(1 < stateList.size()) {
		nextState = getOptionPropertyValue(&(stateList.data()[1]), stateList.size() - 1, nextStates, (MyState*)nullptr);
	}
	if(!nextState) {
		// If Next state name specified in nextState combo box does not match existing states,
		// check if new state name is specified.
		auto nextStateName = getStringPropertyValue(nextStates);
		if(!nextStateName.empty()) {
			// Create new state and set it as next state.
			// Master state should be nullptr or selected from stateList list box including top state(current state)
			auto masterState = getOptionPropertyValue(stateList, masterStates, (MyState*)nullptr);
			e->nextState = new MyState(*context, nextStateName, masterState);
			e->nextState.p->callExitOnShutDown = getBoolPropertyValue(callExitOnShutdownProperty);

			// Clear next state name.
			nextStates->SetValue(_T(""));
		}
	} else {
		// Next state if selected from stateList list box.
		e->nextState = nextState;
	}

	return e;
}

// Called when next state is changed.
// Determines either master state and callExitOnShutDown are visible or not
// depending on value of next state.
void CEventProperties::updateNextStates()
{
	auto enable = FALSE;
	auto nextStateName = getStringPropertyValue(nextStates);
	if(!nextStateName.empty() && !stateList.empty()) {
		auto nextState = getOptionPropertyValue(stateList, nextStates, (MyState*)nullptr);
		// If next state is selected from state list, specifying master state and callExitOnShutDown is not necessary.
		// See createEvent() method.
		enable = nextState ? FALSE : TRUE;
	}
	masterStates->Enable(enable);
	callExitOnShutdownProperty->Enable(enable);

	RedrawWindow();
}

std::tstring CEventProperties::getStringPropertyValue(const CMFCPropertyGridProperty * property) const
{
	auto& var = property->GetValue();
	return (LPCTSTR)((bstr_t)var);
}

bool CEventProperties::getBoolPropertyValue(const CMFCPropertyGridProperty * property) const
{
	auto var = property->GetValue();
	return var.boolVal ? true : false;
}

template<typename T>
void CEventProperties::setOptionProperty(const OptionItem<T>* optionItems, size_t optionItemCount, CMFCPropertyGridProperty* property)
{
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
T CEventProperties::getOptionPropertyValue(const OptionItem<T>* optionItems, size_t optionItemCount, const CMFCPropertyGridProperty* property, T defaultValue) const
{
	auto str = getStringPropertyValue(property);
	for(size_t i = 0; i < optionItemCount; i++) {
		auto& optionItem = optionItems[i];
		if(str == optionItem.name) return optionItem.value;
	}
	return defaultValue;
}

template<typename T, int optionItemCount>
T CEventProperties::getOptionPropertyValue(const OptionItem<T> (&optionItems)[optionItemCount], const CMFCPropertyGridProperty* property, T defaultValue) const
{
	return getOptionPropertyValue(optionItems, optionItemCount, property, defaultValue);
}

template<typename T>
T CEventProperties::getOptionPropertyValue(const std::vector<OptionItem<T>>& optionItems, const CMFCPropertyGridProperty* property, T defaultValue) const
{
	return getOptionPropertyValue(optionItems.data(), optionItems.size(), property, defaultValue);
}

BOOL CEventProperties::CList::OnUpdateValue()
{
	auto ret = CMFCPropertyGridProperty::OnUpdateValue();

	if(onValueUpdated) {
		onValueUpdated();
	}
	return ret;
}

/*static*/ const CEventProperties::OptionItem<HRESULT> CEventProperties::CHResultProperty::VALUE_LIST[] = {
#define DEF_ITEM(x) { _T(#x), x }
	DEF_ITEM(S_OK),
	DEF_ITEM(S_FALSE),
	DEF_ITEM(E_ABORT),
	DEF_ITEM(E_UNEXPECTED),
#undef DEF_ITEM
};

CEventProperties::CHResultProperty::CHResultProperty(CEventProperties* owner, const CString & strName, LPCTSTR lpszDescr)
	: CMFCPropertyGridProperty(strName, _T(""), lpszDescr), owner(owner)
{
	owner->setOptionProperty<HRESULT>(VALUE_LIST, this);
	SetValue(VALUE_LIST[0].name);
}

HRESULT CEventProperties::CHResultProperty::getSelectedValue() const
{
	return owner->getOptionPropertyValue<HRESULT>(VALUE_LIST, this, VALUE_LIST[0].value);
}
