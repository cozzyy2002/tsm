#include "stdafx.h"

#include "StateMachine.NET.h"
#include "NativeObjects.h"

//-------------- Managed Context class. --------------------//
using namespace tsm_NET;
using namespace tsm_NET::common;

void Context::construct(bool isAsync)
{
	m_nativeContext = new native::Context(this, isAsync);
}

Context::~Context()
{
	this->!Context();
}

Context::!Context()
{
	if(m_nativeContext) {
		delete m_nativeContext;
		m_nativeContext = nullptr;
	}
}

bool Context::isAsync()
{
	return m_nativeContext->isAsync();
}

HResult Context::setup(State^ initialState, Event^ event)
{
	return (HResult)m_nativeContext->setup(getNative(initialState), getNative(event));
}

HResult Context::shutdown(TimeSpan timeout)
{
	return (HResult)m_nativeContext->shutdown((DWORD)timeout.TotalMilliseconds);
}

HResult Context::triggerEvent(Event^ event)
{
	return (HResult)m_nativeContext->triggerEvent(getNative(event));
}

HResult Context::handleEvent(Event^ event)
{
	return (HResult)m_nativeContext->handleEvent(getNative(event));
}

HResult Context::waitReady(TimeSpan timeout)
{
	return (HResult)m_nativeContext->waitReady((DWORD)timeout.TotalMilliseconds);
}

State^ Context::getCurrentState()
{
	return getManaged(m_nativeContext->getCurrentState());
}

//-------------- Managed State class. --------------------//
State::State(State^ masterState)
{
	m_nativeState = new native::State(this, masterState);
	m_nativeState->AddRef();
}

State::~State()
{
	this->!State();
}

State::!State()
{
	if(m_nativeState)
	{
		m_nativeState->Release();
		m_nativeState = nullptr;
	}
}

HRESULT State::handleEventCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState** nextState)
{
	State^ _nextState = nullptr;
	auto hr = handleEvent(getManaged((native::Context*)context), getManaged((native::Event*)event), _nextState);
	if(_nextState) {
		*nextState = getNative(_nextState);
	}
	return (HRESULT)hr;
}

HRESULT State::entryCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* previousState)
{
	return (HRESULT)entry(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)previousState));
}

HRESULT State::exitCallback(tsm::IContext* context, tsm::IEvent* event, tsm::IState* nextState)
{
	return (HRESULT)entry(getManaged((native::Context*)context), getManaged((native::Event*)event), getManaged((native::State*)nextState));
}

State^ State::getMasterState()
{
	return getManaged(m_nativeState->getMasterState());
}

//State^ State::getSubState()
//{
//	auto subState = m_nativeState->getSubState();
//	return subState ? subState->get() : nullptr;
//}

bool State::isSubState()
{
	return m_nativeState->isSubState();
}

//-------------- Managed Event class. --------------------//
Event::Event()
{
	m_nativeEvent = new native::Event(this);
	m_nativeEvent->AddRef();
}

Event::~Event()
{
	this->!Event();
}

Event::!Event()
{
	if(m_nativeEvent)
	{
		m_nativeEvent->Release();
		m_nativeEvent = nullptr;
	}
}

HRESULT Event::preHandleCallback(tsm::IContext* context)
{
	return (HRESULT)preHandle(getManaged((native::Context*)context));
}

HRESULT Event::postHandleCallback(tsm::IContext* context, HRESULT hr)
{
	return (HRESULT)postHandle(getManaged((native::Context*)context), (HResult)hr);
}
