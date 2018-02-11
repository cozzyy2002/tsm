#include <StateMachine/stdafx.h>
#include <StateMachine/Unknown.h>

#include <iostream>

using namespace tsm;

Unknown::Unknown() : m_cRef(0)
{
}


Unknown::~Unknown()
{
}

tstring Unknown::toString() const
{
	CA2T str(typeid(*this).name());
	return (LPCTSTR)str;
}

ULONG STDMETHODCALLTYPE Unknown::AddRef(void)
{
	auto cRef = InterlockedIncrement(&m_cRef);
	return cRef;
}

ULONG STDMETHODCALLTYPE Unknown::Release(void)
{
	auto cRef = InterlockedDecrement(&m_cRef);
	if(0 == cRef) delete this;
	return cRef;
}
