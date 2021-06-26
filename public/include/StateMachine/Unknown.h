#pragma once

namespace tsm {

template<class T>
class UnknownImpl
{
public:
	UnknownImpl(T* obj, LPCQITAB pqit = nullptr);

	HRESULT QueryInterface(REFIID riid, void** ppvObject);
	ULONG AddRef(void);
	ULONG Release(void);

	// For UnitTest
	ULONG& getCRef() { return m_cRef; }

protected:
	T* m_obj;
	ULONG m_cRef;
	LPCQITAB m_pqit;

	static const QITAB defaultQITab[];
};

class tsm_STATE_MACHINE_EXPORT Unknown : public IUnknown
{
public:
	Unknown() : m_impl(this) {}
	virtual ~Unknown() {}

	virtual std::tstring toString() const;

#pragma region Implementation of IUnknown
	/**
	 * QueryInterface() is not implemented.
	 * If necessary, app should implement this method.
	 */
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) { return m_impl.QueryInterface(riid, ppvObject); }
	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return m_impl.AddRef(); }
	virtual ULONG STDMETHODCALLTYPE Release(void) { return m_impl.Release(); }
#pragma endregion

protected:
#pragma warning(push)
#pragma warning(disable:4251)
	UnknownImpl<Unknown> m_impl;
#pragma warning(pop)
};

template<class T>
const QITAB UnknownImpl<T>::defaultQITab[] = {
	QITABENT(T, IUnknown),
	{ 0 }
};

template<class T>
UnknownImpl<T>::UnknownImpl(T* obj, LPCQITAB pqit /*= nullptr*/)
	: m_obj(obj), m_pqit(pqit ? pqit : defaultQITab), m_cRef(0)
{
}

template<class T>
HRESULT UnknownImpl<T>::QueryInterface(REFIID riid, void** ppvObject)
{
	return QISearch(m_obj, m_pqit, riid, ppvObject);
}

template<class T>
ULONG UnknownImpl<T>::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

template<class T>
ULONG UnknownImpl<T>::Release()
{
	auto cRef = InterlockedDecrement(&m_cRef);
	if(cRef == 0) {
		delete m_obj;
	}
	return cRef;
}

}
