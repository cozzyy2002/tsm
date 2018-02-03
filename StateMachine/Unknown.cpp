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

HRESULT STDMETHODCALLTYPE Unknown::QueryInterface(REFIID riid, void ** ppvObject)
{
	return QISearch(this, getQUITABs(), riid, ppvObject);
}

ULONG STDMETHODCALLTYPE Unknown::AddRef(void)
{
	auto cRef = InterlockedIncrement(&m_cRef);
	std::cout << typeid(*this).name() << "::AddRef: Reference count=" << m_cRef << std::endl;
	return cRef;
}

ULONG STDMETHODCALLTYPE Unknown::Release(void)
{
	auto cRef = InterlockedDecrement(&m_cRef);
	std::cout << typeid(*this).name() << "::Release: Reference count=" << m_cRef << std::endl;
	if(0 == cRef) delete this;
	return cRef;
}

LPCQITAB Unknown::getQUITABs() const
{
	static const QITAB qitabs[] = {
		QITABENT(Unknown, IUnknown),
		{ 0 }
	};

	return qitabs;
}
