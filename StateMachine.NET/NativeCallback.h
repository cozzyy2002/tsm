#pragma once

using System::Runtime::InteropServices::Marshal;

namespace native
{
/*
	Callback template class.
	This class is used to avoid `Cannot pass a GCHandle across AppDomains` exception
	when method of managed class is called by native(unmanaged) class in worker thread.

	How to use this class.

	1. Define delegate(class D), callback signature(class C) and callback method in managed class(class M).
		delegate HRESULT XxxDelegate(...);				// delegate
		typedef HRESULT (__stdcall *XxxCallback)(...);	// callback signature
		HRESULT xxxCallback(...) { method body }		// callback method

	2. Declare member variable of this class in native class.
		Callback<XxxDelegate, XxxCallback> m_xxxCallback;

	3. Initialize the member variable in the constructor of native class.
		m_xxxCallback(gcnew XxxDelegate(managedObject, &ManagedClass::xxxCallback))

	4. Callback from native class.
		m_xxxCallback(...);

	NOTE:
		Parameters and return value should be native(unmanaged) type.

	See http://lambert.geek.nz/2007/05/unmanaged-appdomain-callback/.
*/
template<class D, class C>
class Callback
{
public:
	Callback(D^ del) : del(del) {
		callback = (C)Marshal::GetFunctionPointerForDelegate(del).ToPointer();
	}

	operator C() { return callback; }

protected:
	gcroot<D^> del;
	C callback;
};

}
