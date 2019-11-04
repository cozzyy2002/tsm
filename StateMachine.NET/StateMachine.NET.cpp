#include "stdafx.h"

#include "StateMachine.NET.h"
#include "NativeObjects.h"

//-------------- Managed Context class. --------------------//
using namespace tsm_NET;
using namespace tsm_NET::common;

Context::Context()
{
	m_nativeContext = new native::Context(this);
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

HRESULT State::handleEventCallback(native::Context* context, native::Event* event, native::State** nextState)
{
	State^ _nextState = nullptr;
	auto hr = handleEvent(getManaged(context), getManaged(event), _nextState);
	if(_nextState) {
		*nextState = getNative(_nextState);
	}
	return (HRESULT)hr;
}

HRESULT State::entryCallback(native::Context* context, native::Event* event, native::State* previousState)
{
	return (HRESULT)entry(getManaged(context), getManaged(event), getManaged(previousState));
}

HRESULT State::exitCallback(native::Context* context, native::Event* event, native::State* nextState)
{
	return (HRESULT)entry(getManaged(context), getManaged(event), getManaged(nextState));
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

HRESULT Event::preHandleCallback(native::Context* context)
{
	return (HRESULT)preHandle(getManaged(context));
}

HRESULT Event::postHandleCallback(native::Context* context, HRESULT hr)
{
	return (HRESULT)postHandle(getManaged(context), (HResult)hr);
}
