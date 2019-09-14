#include "stdafx.h"

#include "StateMachine.NET.h"
#include "NativeObjects.h"

template<class C>
typename C::NativeType* getNative(C^ managed)
{
	return managed ? managed->get() : nullptr;
}

//-------------- Managed Context class. --------------------//
using namespace tsm_NET;

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
	return m_nativeContext->getCurrentState()->get();
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

State^ State::getMasterState()
{
	auto masterState = m_nativeState->getMasterState();
	return masterState ? masterState->get() : nullptr;
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
