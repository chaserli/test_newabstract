
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
	virtual void Execute(WWKey eInput) const override
	{
		std::wstring toprint = std::to_wstring((int)eInput);

		MessageListClass::Instance->PrintMessage(toprint.c_str());
	}
};

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
