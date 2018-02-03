#include "stdafx.h"

#include "Mocks.h"

bool TestUnknown::deleted() const
{
	return (rcRef == 0);
}

/*
 * Implementation of IUnknown::Release()
 *
 * This method does not delete this object even if reference count reaches zero to:
 *   Be created in stack.
 *   check where the object is deleted by calling deleted() method.
 */
ULONG TestUnknown::Release(IUnknown* _this)
{
	rcRef = InterlockedDecrement(&rcRef);
	if((LONG)rcRef < 0) {
		ADD_FAILURE() << typeid(*_this).name() << ": Invalid reference count: " << (LONG)rcRef;
	}
	return rcRef;
}
