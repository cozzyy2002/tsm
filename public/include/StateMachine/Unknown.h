#pragma once

namespace tsm {

#if defined(UNICODE)
using tstring = std::wstring;
#else
using tstring = std::string;
#endif

template<class T>
class Unknown : public T
{
public:
	Unknown() : m_cRef(0) {}
	virtual ~Unknown() {}

	virtual tstring toString() {
		CA2T str(typeid(*this).name());
		return (LPCTSTR)str;
	}

#pragma region Implementation of IUnknown
	/**
	 * QueryInterface() returns only IUnknown object.
	 * If necessary, app should implement this method.
	 */
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) {
		if(!ppvObject) { return E_POINTER; }

		*ppvObject = nullptr;
		if(IID_IUnknown == riid) {
			*ppvObject = static_cast<IUnknown*>(this);
			this->AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) {
		auto cRef = InterlockedIncrement(&m_cRef);
		return cRef;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) {
		auto cRef = InterlockedDecrement(&m_cRef);
		if(0 == cRef) delete this;
		return cRef;
	}

#pragma endregion

protected:
	ULONG m_cRef;
};

}
