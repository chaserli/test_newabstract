#include "ShitClass.h"
#include <YRCom.h>
#include <Helpers/Macro.h>

DEFINE_HOOK(0x55B6B3, LogicClass_Update_Shit, 0x5)
{
	for (auto shit : ShitClass::Array)
		shit->Update();

	return 0;
}

DEFINE_HOOK(0x67E12A, SaveGame_BeforeST, 0x6)
{
	GET(IStream*, pStm, ESI);

	if (FAILED(ShitClass::SaveArray(pStm)))
		return 0x67E17C;

	return 0;
}

DEFINE_HOOK(0x67F3B7, LoadGame_BeforeST, 0x5)
{
	GET(IStream*, pStm, ESI);

	if (FAILED(ShitClass::LoadArray(pStm)))
		return 0x67F48E;

	return 0;
}

DEFINE_HOOK(0x67E68A, PostSwizzleShit_CuzAllVacancyTakenByKkuCykaBlyat, 0x5)
{
	ShitClass::ReAttachAll();
	return 0;
}

DEFINE_HOOK(0x6BD6B1, WinMain_RegisterShit, 0x5)
{
	auto pFactory = GameCreate<TClassFactory<ShitClass>>();

	DWORD dwRegister;
	if (SUCCEEDED(CoRegisterClassObject(
		__uuidof(ShitClass), pFactory, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister
	)))
		Game::COMClasses->AddItem(dwRegister);

	return 0;
}

DEFINE_HOOK(0x685241, ClearLotsOfShits_IncludingMyShit, 0x6)
{
	while (!ShitClass::Array.empty())
		delete (*ShitClass::Array.rbegin());
	return 0;
}
