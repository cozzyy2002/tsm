#include "stdafx.h"

#include "Mocks.h"

// Mock is created in stack not heap, so it does not delete itself.
ULONG STDMETHODCALLTYPE MockEvent::Release(void)
{
	auto cRef = InterlockedDecrement(&m_cRef);
	return cRef;
}

// Mock is created in stack not heap, so it does not delete itself.
ULONG STDMETHODCALLTYPE MockState::Release(void)
{
	auto cRef = InterlockedDecrement(&m_cRef);
	return cRef;
}
