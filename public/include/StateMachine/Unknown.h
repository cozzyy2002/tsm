#pragma once

namespace tsm {

#if defined(UNICODE)
using tstring = std::wstring;
#else
using tstring = std::string;
#endif

class Unknown : public IUnknown
{
public:
	Unknown();
	virtual ~Unknown();

	virtual tstring toString() const;

#pragma region Implementation of IUnknown
	/**
	 * QueryInterface() is not implemented.
	 * If necessary, app should implement this method.
	 */
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) { return E_NOTIMPL; }
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);
#pragma endregion

protected:
	ULONG m_cRef;
};

}
