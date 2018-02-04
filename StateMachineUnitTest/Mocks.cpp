#include "stdafx.h"

#include "Mocks.h"

TestUnknown::~TestUnknown()
{
	if(rcRef != 0) {
		ADD_FAILURE() << className << ": Reference count is not zero at destruction: " << (LONG)rcRef;
	}
}

bool TestUnknown::deleted() const
{
	return (rcRef == 0);
}

/*
 * Implementation of IUnknown::Release()
 *
 * This method does not delete this object even if reference count reaches zero to:
 *   Be created in stack.
 *   check whether the object is deleted by calling deleted() method.
 */
ULONG TestUnknown::Release()
{
	LONG cRef = InterlockedDecrement(&rcRef);
	if(cRef < 0) {
		ADD_FAILURE() << className << ": Invalid reference count: " << cRef;
	}
	return rcRef;
}

void TestUnknown::setObject(IUnknown * _this)
{
	className = typeid(*_this).name();
}
