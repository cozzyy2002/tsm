#include "stdafx.h"

#include "MyEvent.h"

HRESULT MyEvent::preHandle(MyContext* context)
{
	m_logger->log(_T("%s::preHandle(): HRESULT=0x%p"), MyObject::toString(), hrPreHandle);

	return hrPreHandle;
}

ULONG MyEvent::Release(void)
{
	if(1 == m_cRef) {
		m_logger->log(_T("Deleting %s"), MyObject::toString());
	}
	return Unknown::Release();
}
