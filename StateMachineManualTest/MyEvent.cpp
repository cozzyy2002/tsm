#include "stdafx.h"

#include "MyEvent.h"

HRESULT MyEvent::preHandle(MyContext* context)
{
	m_logger->log(_T("%s::preHandle(): HRESULT=0x%p"), MyObject::toString(), hrPreHandle);

	return hrPreHandle;
}

HRESULT MyEvent::postHandle(MyContext * context, HRESULT hr)
{
	m_logger->log(_T("%s::postHandle(hr=0x%p): HRESULT=0x%p"), MyObject::toString(), hr, hrPreHandle);

	return hrPostHandle;
}

ULONG MyEvent::Release(void)
{
	if(1 == m_cRef) {
		m_logger->log(_T("Deleting %s"), MyObject::toString());
	}
	return Unknown::Release();
}
