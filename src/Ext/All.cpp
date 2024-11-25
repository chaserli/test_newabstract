#include "ShitClass.h"
#include <Helpers/Macro.h>

DEFINE_HOOK(0x55B6B3, LogicClass_Update_Shit, 0x5)
{
	for (auto shit : ShitClass::Array)
		shit->Update();

	return 0;
}

DEFINE_HOOK(0x67E12A, SaveGame_BeforeST, 0x6)
{
	GET(IStream *, pStm, ESI);

	if (FAILED(ShitClass::SaveArray(pStm)))
		return 0x67E17C;

	return 0;
}

DEFINE_HOOK(0x67F3B7, LoadGame_BeforeST, 0x5)
{
	GET(IStream *, pStm, ESI);

	if (FAILED(ShitClass::LoadArray(pStm)))
		return 0x67F48E;

	return 0;
}

DEFINE_HOOK(0x67E68A, PostSwizzleShit_CuzAllVacancyTakenByKkuCykaBlyat, 0x5)
{
	ShitClass::ReAttachAllOnPostSwizzle();
	return 0;
}

template <typename T>
	requires std::derived_from<T, IPersistStream>
class T_IClassFactory : public IClassFactory
{
public:
	T_IClassFactory()
	{
		this->nRefCount = 0;
	}

	virtual HRESULT __stdcall QueryInterface(const IID &riid, void **ppvObject) override
	{
		if (!ppvObject)
			return E_POINTER;

		*ppvObject = nullptr;

		if (riid == __uuidof(IUnknown))
			*ppvObject = static_cast<IUnknown *>(this);

		if (riid == __uuidof(IClassFactory))
			*ppvObject = static_cast<IClassFactory *>(this);

		if (!*ppvObject)
			return E_NOINTERFACE;

		this->AddRef();

		return S_OK;
	}

	virtual ULONG __stdcall AddRef() override
	{
		return InterlockedIncrement(&this->nRefCount);
	}

	virtual ULONG __stdcall Release() override
	{
		int nNewRef = InterlockedIncrement(&this->nRefCount);
		if (!nNewRef)
			delete (this);
		return nNewRef;
	}

	virtual HRESULT __stdcall CreateInstance(IUnknown *pUnkOuter, const IID &riid, void **ppvObject) override
	{
		if (!ppvObject)
			return E_INVALIDARG;

		*ppvObject = nullptr;
		if (pUnkOuter)
			return CLASS_E_NOAGGREGATION;

		T *pThis = new T();
		if (!pThis)
			return E_OUTOFMEMORY;

		HRESULT hr = pThis->QueryInterface(riid, ppvObject);

		if (FAILED(hr))
			delete pThis;

		return hr;
	}

	virtual HRESULT __stdcall LockServer(BOOL fLock) override
	{
		this->nRefCount += fLock ? 1 : -1;

		return S_OK;
	}

	static void *operator new(size_t size)
	{
		return YRMemory::Allocate(size);
	}

	static void operator delete(void *p)
	{
		YRMemory::Deallocate(p);
	}

private:
	LONG nRefCount{0};
};

template <typename Derived>
	requires std::derived_from<Derived, AbstractClass>
void RegisterTClassFactory()
{
	auto pFactory = new T_IClassFactory<Derived>();

	DWORD dwRegister;
	if (SUCCEEDED(CoRegisterClassObject(
			__uuidof(Derived), pFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister)))
		Game::COMClasses->AddItem(dwRegister);
}

DEFINE_HOOK(0x6BD6B1, WinMain_RegisterShit, 0x5)
{
	RegisterTClassFactory<ShitClass>();
	return 0;
}

DEFINE_HOOK(0x685241, ClearLotsOfShits_IncludingMyShit, 0x6)
{
	ShitClass::ClearAllOnExit();
	return 0;
}
