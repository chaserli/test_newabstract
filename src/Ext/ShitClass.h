#pragma once
#include <HouseClass.h>
#include <Unsorted.h>
#include <UnitClass.h>
#include <vector>

class ShitTypeClass;

class DECLSPEC_UUID("8DCDFF5D-E5EE-42D1-A33D-53980FE2A0D4")
	ShitClass final : public AbstractClass
{
public:
	static constexpr AbstractType AbsID = AbstractType(1145);
	static std::vector<ShitClass*> Array;
public:
	explicit ShitClass(UnitClass* owner, UnitTypeClass* image);

	explicit ShitClass(noinit_t);
	explicit ShitClass();

	//IUnknown
	virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;

	virtual STDMETHODIMP_(ULONG) AddRef() override
	{
		//return InterlockedIncrement(&this->RefCount);
		return 1;
	}

	virtual STDMETHODIMP_(ULONG) Release() override
	{
		/*
		ULONG count = InterlockedDecrement(&this->RefCount);
		if (count == 0) {
			delete this;
		}
		return count;
		*/
		return 1;
	}

	//IPersist
	virtual STDMETHODIMP GetClassID(CLSID* pClassID) override
	{
		if (!pClassID) return E_POINTER;
		*pClassID = __uuidof(ShitClass);
		return S_OK;
	}

	//IPersistStream
	virtual STDMETHODIMP IsDirty() override
	{
		return this->Dirty == false ? S_OK : S_FALSE;
	}

	virtual STDMETHODIMP Load(IStream* pStm) override;

	virtual STDMETHODIMP Save(IStream* pStm, BOOL fClearDirty) override;

	virtual STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize) override
	{
		if (!pcbSize) return E_POINTER;
		pcbSize->QuadPart = sizeof(ShitClass) + 4;
		return S_OK;
	}


	//IRTTITypeInfo
	virtual AbstractType __stdcall What_Am_I() const override { return ShitClass::WhatAmI(); }
	virtual int __stdcall Fetch_ID() const override { return this->UniqueID; }
	virtual void __stdcall Create_ID() override;

	//INoticeSink
	virtual bool __stdcall INoticeSink_Unknown(DWORD dwUnknown) override R0;

	//INoticeSource
	virtual void __stdcall INoticeSource_Unknown() override RX;

	// AbstractClass
	virtual ~ShitClass() noexcept;

	virtual void Init() override {}

	virtual void PointerExpired(AbstractClass*, bool) override;

	virtual AbstractType WhatAmI() const override { return ShitClass::AbsID; }

	virtual int Size() const override { return sizeof(ShitClass); }

	virtual void ComputeCRC(CRCEngine& crc) const override;

	virtual int GetOwningHouseIndex() const override { return this->OwnerUnit->Owner->ArrayIndex; }
	virtual HouseClass* GetOwningHouse() const override { return this->OwnerUnit->Owner; }
	virtual int GetArrayIndex() const override { return this->ArrayIndex; }

	virtual bool IsDead() const override { return this->OwnerUnit->IsDead(); }

	virtual CoordStruct* GetCoords(CoordStruct* pCrd) const override
	{
		*pCrd = this->Location;
		return pCrd;
	}

	virtual CoordStruct* GetDestination(CoordStruct* pCrd, TechnoClass* pDocker = nullptr) const override
	{
		return this->OwnerUnit->GetDestination(pCrd,pDocker);
	}
	virtual bool IsOnFloor() const override { return this->OwnerUnit->IsOnFloor(); }
	virtual bool IsInAir() const override { return this->OwnerUnit->IsInAir(); }
	virtual CoordStruct* GetCenterCoords(CoordStruct* pCrd) const override { return ShitClass::GetCoords(pCrd); }
	virtual void Update() override;

public:
	size_t ArrayIndex;
	UnitClass* OwnerUnit;
	UnitTypeClass* Image;
	CoordStruct Location;
	DECLARE_PROPERTY(FacingClass, Facing);// Cyka noinit_t ne marche pas blyat
	DECLARE_PROPERTY(CDTimerClass, ROFTimer);
	int LastFrameInLogic;
	int AwakenFrame;
	int CurrentWeaponIndex;
	int HowManyTimesIFuckingFired;
	AnimClass* AttachedAnim;
	AnimClass* DirectionAnim;
	DirType OwnerDirection;
	CoordStruct Offset;

private:
	void AddTracking();
	bool CheckRemoved();

public:
	// starkku took the vacancy blyat
	void AttachToUnit(UnitClass* self)
	{
		*(WORD*)((DWORD)self + 105) = HIWORD((DWORD)this);
		*(WORD*)((DWORD)self + 117) = LOWORD((DWORD)this);
	}

	static void ReAttachAllOnPostSwizzle()
	{
		for (auto shit : ShitClass::Array)
			shit->AttachToUnit(shit->OwnerUnit);
	}

	static void ClearAllOnExit()
	{
		while (!ShitClass::Array.empty())
			delete (*ShitClass::Array.rbegin());
	}
	static ShitClass* __fastcall GetShit(ObjectClass const* self)
	{
		WORD high = *(WORD*)((DWORD)self + 105);
		WORD low = *(WORD*)((DWORD)self + 117);
		return reinterpret_cast<ShitClass*>((DWORD)high << 16 | low);
	}

	static void __fastcall ClearShit(ObjectClass* self)
	{
		*(WORD*)((DWORD)self + 105) = 0;
		*(WORD*)((DWORD)self + 117) = 0;
	}


	void DrawAsVXL(RectangleStruct*, Point2D*, Matrix3D*, DWORD);
	void OnFire(BulletClass*);


	static HRESULT LoadArray(IStream*);
	static HRESULT SaveArray(IStream*);
};
