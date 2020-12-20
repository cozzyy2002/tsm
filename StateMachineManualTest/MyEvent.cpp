#include "stdafx.h"

#include "MyEvent.h"

MyEvent::MyEvent(ILogger& logger, const std::tstring& name, int priority)
	: MyObject(name.c_str(), &logger), Event(priority)
{
	hrPreHandle = hrPostHandle = hrHandleEvent = hrEntry = hrExit = S_OK;
}

MyEvent::~MyEvent()
{
	m_logger->log(_T("Deleting %s"), MyObject::toString());
}

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
