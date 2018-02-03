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
ULONG TestUnknown::Release()
{
	rcRef = InterlockedDecrement(&rcRef);
	if((LONG)rcRef < 0) {
		ADD_FAILURE() << "Invalid reference count: " << rcRef;
	}
	return rcRef;
}
