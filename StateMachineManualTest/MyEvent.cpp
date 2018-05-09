#include "stdafx.h"

#include "MyEvent.h"

ULONG MyEvent::Release(void)
{
	if(1 == m_cRef) {
		m_logger->log(_T("Deleting %s"), MyObject::toString());
	}
	return Unknown::Release();
}
