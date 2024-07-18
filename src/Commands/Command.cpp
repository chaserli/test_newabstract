
#include <StringTable.h>
#include <CommandClass.h>
#include <MessageListClass.h>
#include <string>

class NothingCommand final : public CommandClass
{
public:
	virtual const char* GetName() const override { return "DoNothing"; }
	virtual const wchar_t* GetUIName() const override { return L"Don't Do Fucking Anything"; }
	virtual const wchar_t* GetUICategory() const override { return L"Blyat"; }
	virtual const wchar_t* GetUIDescription() const override { return L"Nothing is done"; }
	static bool Clicked;
	virtual void Execute(WWKey eInput) const override
	{
		MessageListClass::Instance->PrintMessage(L"Why?");
		const auto openweb = reinterpret_cast<char(__fastcall*)(const char*, BOOL)>(0x77DC90);
		if (!Clicked)
		{
			openweb("https://github.com/chaserli/test_newabstract", 1);
			Clicked = true;
		}
		else {
			openweb("https://github.com/Phobos-developers/Phobos", 1);
			Clicked = false;
		}
	}
};
bool NothingCommand::Clicked = false;

template <typename T>
void MakeCommand()
{
	T* command = GameCreate<T>();
	CommandClass::Array->AddItem(command);
};

DEFINE_HOOK(0x533066, CommandClassCallback_RegisterBlyat, 0x6)
{
	MakeCommand<NothingCommand>();

	return 0;
}
