#include "ShitClass.h"
#include <ScenarioClass.h>
#include <CRC.h>
#include <Utilities/Stream.h>
#include <Utilities/Debug.h>
#include <Notifications.h>
#include <Helpers/Macro.h>
#include <AnimClass.h>
#include <map>
#include <format>
#include <BulletClass.h>
#include <LaserDrawClass.h>
#include <VocClass.h>

std::vector<ShitClass*> ShitClass::Array;
/*
namespace OtherShits
{
	static constexpr reference<PointerExpiredNotification, 0xB0F698> const Invalid{};
}
*/
// IPersistStream

STDMETHODIMP ShitClass::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_IUnknown || riid == IID_IPersist || riid == IID_IPersistStream)
	{
		*ppvObject = static_cast<IPersistStream*>(this);
		this->AddRef();
		return S_OK;
	}

	if (riid == IIDs::IRTTITypeInfo())
	{
		*ppvObject = static_cast<IRTTITypeInfo*>(this);
		this->AddRef();
		return S_OK;
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}

STDMETHODIMP ShitClass::Load(IStream* pStm)
{
	auto hr = reinterpret_cast<HRESULT(__stdcall*)(AbstractClass*, IStream*)>(0x410380)(this, pStm);
	if (SUCCEEDED(hr))
	{
		new (this) ShitClass(noinit_t());
		//Facing broken blyat
		MSwizzle(this->OwnerUnit);
		MSwizzle(this->Image);
		MSwizzle(this->AttachedAnim);
		MSwizzle(this->DirectionAnim);
	}
	return hr;
}

STDMETHODIMP ShitClass::Save(IStream* pStm, BOOL fClearDirty)
{
	return reinterpret_cast<HRESULT(__stdcall*)(AbstractClass*, IStream*, BOOL)>(0x410320)(this, pStm, fClearDirty);
}

// IRTTIInfo
void ShitClass::Create_ID()
{
	if (auto scene = ScenarioClass::Instance())
		this->UniqueID = (++scene->UniqueID);
	else
		this->UniqueID = 0;
}

// AbstractClass

void ShitClass::AddTracking()
{
	this->unknown_18 = 0;
	this->RefCount = 0;
	this->Dirty = false;
	this->UniqueID = DWORD(-5);
	this->AbstractFlags = AbstractFlags::None;
	PointerExpiredNotification::NotifyInvalidObject->Add(this);
	PointerExpiredNotification::NotifyInvalidAnim->Add(this);
	this->ArrayIndex = ShitClass::Array.size();
	ShitClass::Array.push_back(this);
}

ShitClass::ShitClass(UnitClass* owner, UnitTypeClass* image) :AbstractClass{ noinit_t() }
, OwnerUnit{ owner }
, Image{ image }
, Location{ 0,0,0 }
, Facing{ 2 }
, ROFTimer{}
, LastFrameInLogic{ -1 }
, AwakenFrame{ -1 }
, CurrentWeaponIndex{ -1 }
, AttachedAnim{ nullptr }
, DirectionAnim{ nullptr }
, OwnerDirection{ DirType::North }
, Offset{ -300,0,0 }
, NowTransform{}
, HowManyTimesIFuckingFired{ 0 }
{
	AddTracking();
	this->AttachToUnit(owner);
	this->Facing.SetROT(image->ROT);
	Debug::Log("Making shit %d for %s with image of %s\n", this->ArrayIndex, this->OwnerUnit->Type->ID, this->Image->ID);
}

ShitClass::ShitClass(noinit_t) :AbstractClass{ noinit_t() }, Facing{ noinit_t() } {}

ShitClass::ShitClass() : ShitClass(noinit_t())
{
	AddTracking();
	Debug::Log("Making empty shit cykablyat!\n");
}

ShitClass::~ShitClass()
{
	Debug::Log("deleting shit[%x]... \n", this->ArrayIndex);
	if (this->AttachedAnim)
		this->AttachedAnim->UnInit();
	if (this->DirectionAnim)
		this->DirectionAnim->UnInit();
	ShitClass::Array.erase(std::remove(ShitClass::Array.begin(), ShitClass::Array.end(), this), ShitClass::Array.end());
	PointerExpiredNotification::NotifyInvalidObject->Remove(this);
	PointerExpiredNotification::NotifyInvalidAnim->Remove(this);
}

void ShitClass::ComputeCRC(CRCEngine& crc) const
{
	if (this->OwnerUnit)
		crc(this->OwnerUnit->UniqueID);
	crc(this->Location.X);
	crc(this->Location.Y);
	crc(this->Location.Z);
	crc(this->ROFTimer.GetTimeLeft());
}

bool ShitClass::CheckRemoved()
{
	auto remove_shit = [this]()
	{
		GameCreate<AnimClass>(RulesClass::Instance->WarpOut, this->Location);
		delete (this);
		return true;
	};

	auto can_removeshit = [this]()->bool
	{
		if (!this->OwnerUnit)
			return true;
		if (!this->OwnerUnit->IsAlive)
			return true;
		if (this->OwnerUnit->Health < 1)
			return true;
		return false;
	};

	if (can_removeshit())
		return remove_shit();

	if (!this->OwnerUnit->IsInLogic)
		return true;

	if (this->AwakenFrame < 0)
	{
		Debug::Log("shit %d awaken !", this->ArrayIndex);
		this->AwakenFrame = Unsorted::CurrentFrame;
	}

	if (this->LastFrameInLogic > 0 && (Unsorted::CurrentFrame - this->LastFrameInLogic > 1))
		Debug::Log("shit %d absent for %d frames\n", this->ArrayIndex, Unsorted::CurrentFrame - this->LastFrameInLogic);

	return false;
}

static const char* DirNames[]
{
	"ARRWN","ARRWNE","ARRWE","ARRWSE","ARRWS","ARRWSW","ARRWW","ARRWNW"
};

void ShitClass::Update()
{
	if (this->CheckRemoved())
		return;
	constexpr int PERIODE = 150;
	this->Offset.Y = 120 * Math::sin(Math::TwoPi * ((Unsorted::CurrentFrame - this->AwakenFrame) % PERIODE) / PERIODE);
	this->Offset.Z = 200 * std::pow(Math::sin(Math::TwoPi * ((Unsorted::CurrentFrame - this->AwakenFrame) % PERIODE) / PERIODE), 2);

	auto lastloc = this->Location;
	{
		this->NowTransform = this->OwnerUnit->Locomotor.GetInterfacePtr()->Draw_Matrix(nullptr);
		this->NowTransform.TranslateX(this->Offset.X);
		double turretRad = this->Facing.Current().GetRadian<32>();
		double bodyRad = this->OwnerUnit->PrimaryFacing.Current().GetRadian<32>();
		this->NowTransform.RotateZ((float)(turretRad - bodyRad));

		auto transl = this->NowTransform.GetTranslation();
		this->Location = this->OwnerUnit->Location + CoordStruct{ (int)transl.X, -(int)transl.Y, (int)transl.Z };
	}

	if (auto tgt = this->OwnerUnit->Target)
	{
		auto& source = this->OwnerUnit->Location;
		auto tgtloc = tgt->GetCoords();
		this->Facing.SetDesired(DirStruct{ Math::atan2(source.Y - tgtloc.Y,tgtloc.X - source.X) });
		if (!this->AttachedAnim || this->AttachedAnim->OwnerObject == this->OwnerUnit)
		{
			if (this->AttachedAnim)
				this->AttachedAnim->UnInit();
			if (auto animtype = AnimTypeClass::Find("ALERTMARK"))
				this->AttachedAnim = GameCreate<AnimClass>(animtype, tgtloc);
		}
	}
	else if (!this->AttachedAnim || this->Location != lastloc)
	{
		if (this->AttachedAnim)
			this->AttachedAnim->UnInit();
		if (auto animtype = AnimTypeClass::Find("ALERTMARK"))
		{
			this->AttachedAnim = GameCreate<AnimClass>(animtype, this->Location);
			this->AttachedAnim->SetOwnerObject(this->OwnerUnit);
		}
	}

	if (!this->Facing.IsRotating() && !this->OwnerUnit->Target)
		this->Facing.SetDesired(DirStruct{ this->Facing.Current().GetRadian<32>() + Math::Pi / 2 });
	auto makeDirAnim = [&]()
	{
		auto name = DirNames[DirStruct{ this->OwnerDirection }.GetFacing<8>()];
		if (auto dirAnim = AnimTypeClass::Find(name))
		{
			this->DirectionAnim = GameCreate<AnimClass>(dirAnim, this->OwnerUnit->Location + CoordStruct{ 0,0,100 });
			this->DirectionAnim->SetOwnerObject(this->OwnerUnit);
		}
	};

	if (!this->OwnerUnit->BunkerLinkedItem)
	{
		if (this->LastFrameInLogic < 0)
		{
			Debug::Log("Making dir anim for Shit[%d] for the first time blyat!\n", this->ArrayIndex);
			this->OwnerDirection = this->OwnerUnit->PrimaryFacing.Current().GetDir();
			makeDirAnim();
		}
		else
		{
			DirType lastDir = this->OwnerDirection;
			this->OwnerDirection = this->OwnerUnit->PrimaryFacing.Current().GetDir();
			if (!this->DirectionAnim)
				makeDirAnim();
			else if (lastDir != this->OwnerDirection)
			{
				this->DirectionAnim->UnInit();
				makeDirAnim();
			}
		}
	}
	this->LastFrameInLogic = Unsorted::CurrentFrame;
}

void ShitClass::PointerExpired(AbstractClass* pAbs, bool removed)
{
	if (this->AttachedAnim == pAbs)
		this->AttachedAnim = nullptr;
	else if (this->DirectionAnim == pAbs)
		this->DirectionAnim = nullptr;
	else if (this->OwnerUnit == pAbs && removed)
	{
		if (!this->OwnerUnit->IsAlive || !this->OwnerUnit->Health)
			this->OwnerUnit = nullptr;
	}
}

HRESULT ShitClass::SaveArray(IStream* pStm)
{
	size_t sz = ShitClass::Array.size();
	auto hr = pStm->Write(&sz, sizeof(sz), nullptr);
	if (SUCCEEDED(hr))
	{
		for (auto const shit : ShitClass::Array)
		{
			hr = OleSaveToStream(shit, pStm);
			if (FAILED(hr))
				return hr;
		}
	}
	return hr;
}

constexpr reference<double, 0xB1D008> const Pixel_Per_Lepton{};
void ShitClass::DrawAsVXL(RectangleStruct* Rectangle, Point2D* CenterPoint, Matrix3D* pMatrix, DWORD dwUnk7)
{
	Matrix3D rot = *pMatrix;
	rot.Translate(this->Offset.as<float>() * Pixel_Per_Lepton);
	rot.RotateZ(this->Facing.Current().GetRadian<32>() - this->OwnerUnit->PrimaryFacing.Current().GetRadian<32>());
	this->OwnerUnit->Draw_A_VXL(
		&this->Image->MainVoxel,
		0,
		-1,
		&this->Image->VoxelTurretBarrelCache,
		Rectangle,
		CenterPoint,
		&rot,
		dwUnk7,
		0x2800,
		0
	);
}

HRESULT ShitClass::LoadArray(IStream* pStm)
{
	size_t sz;
	auto hr = pStm->Read(&sz, sizeof(sz), nullptr);
	if (SUCCEEDED(hr))
	{
		for (size_t i = 0; i < sz; i++)
		{
			LPVOID ppvObj;
			hr = OleLoadFromStream(pStm, IID_IUnknown, &ppvObj);
			if (FAILED(hr))
				return hr;
		}
	}
	return 0;
}

static const char* candidates[]
{
	"HTNK","HTK","MTNK","FV"
};

static const char* imagelists[]
{
	"MGTK","TNKD","HOWI"
};

DEFINE_HOOK(0x735678, UnitClass_CTOR_MakingShit, 0x6)
{
	GET(UnitClass*, self, ESI);

	auto can_shit = [](const char* id)
	{
		for (auto cand : candidates)
			if (_strcmpi(id, cand) == 0)
				return true;
		return false;
	};

	ShitClass::ClearShit(self);
	if (can_shit(self->Type->ID))
		new ShitClass(self, UnitTypeClass::Find(imagelists[0]));

	return 0;
}

DEFINE_HOOK(0x73B7A3, UnitClass_DrawAsVXL_Shit, 0x6)
{
	GET(UnitClass const*, self, EBP);
	LEA_STACK(RectangleStruct*, pRect, STACK_OFFSET(0x1C4, -0x174));
	LEA_STACK(Matrix3D*, pMat, STACK_OFFSET(0x1C4, -0xC0));
	LEA_STACK(Point2D*, ppt, STACK_OFFSET(0x1C4, -0x194));
	GET_STACK(DWORD, fuckyou, STACK_OFFSET(0x1C4, 0x1C));

	if (auto shit = ShitClass::GetShit(self))
		shit->DrawAsVXL(pRect, ppt, pMat, fuckyou);

	return 0;
}

void ShitClass::OnFire(BulletClass* pTraj)
{
	auto pThis = this->OwnerUnit;

	this->HowManyTimesIFuckingFired++;
	if (this->HowManyTimesIFuckingFired % 4 == 3)
		this->Image = UnitTypeClass::Find(imagelists[(this->HowManyTimesIFuckingFired / 4) % std::size(imagelists)]);

	Matrix3D mtx = this->OwnerUnit->Locomotor.GetInterfacePtr()->Draw_Matrix(nullptr);
	{
		mtx.Translate(this->Offset);
		double turretRad = this->Facing.Current().GetRadian<32>();
		double bodyRad = this->OwnerUnit->PrimaryFacing.Current().GetRadian<32>();
		mtx.RotateZ((float)(turretRad - bodyRad));
		mtx.Translate(this->Image->Weapon[0].FLH);
	}
	auto transl = mtx.GetTranslation();
	auto loc = this->OwnerUnit->Location + CoordStruct{ (int)transl.X, -(int)transl.Y, (int)transl.Z };
	GameCreate<LaserDrawClass>(loc
		, pTraj->TargetCoords
		, pThis->Owner->Color
		, pThis->Owner->LaserColor
		, ColorStruct{ 0xff, 0xff, 0xff }
		, pTraj->WeaponType->ROF
		);
}

DEFINE_HOOK(0x7413DD, UnitClass_Fire, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	GET(BulletClass* const, pTraj, EDI);

	if (auto shit = ShitClass::GetShit(pThis))
		shit->OnFire(pTraj);

	return 0;
}
