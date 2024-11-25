
#include <StringTable.h>
#include <CommandClass.h>
#include <MessageListClass.h>
#include <string>
#include <fstream>
#include <sstream>
#include <CCFileClass.h>
#include <filesystem>

class NothingCommand final : public CommandClass
{
public:
	virtual const char *GetName() const override { return "OpenWeb"; }
	virtual const wchar_t *GetUIName() const override { return L"Open Website"; }
	virtual const wchar_t *GetUICategory() const override { return L"Nonsense"; }
	virtual const wchar_t *GetUIDescription() const override { return L"Open a random website"; }
	static bool Clicked;
	virtual void Execute(WWKey eInput) const override
	{
		MessageListClass::Instance->PrintMessage(L"Why?");
		const auto openweb = reinterpret_cast<char(__fastcall *)(const char *, BOOL)>(0x77DC90);
		if (!Clicked)
		{
			openweb("https://github.com/chaserli/test_newabstract", 1);
			Clicked = true;
		}
		else
		{
			openweb("https://github.com/Phobos-developers/Phobos", 1);
			Clicked = false;
		}
	}
};
bool NothingCommand::Clicked = false;

class FileDumpCommand final : public CommandClass
{
public:
	virtual const char *GetName() const override { return "FileDump"; }
	virtual const wchar_t *GetUIName() const override { return L"Copy File"; }
	virtual const wchar_t *GetUICategory() const override { return L"Nonsense"; }
	virtual const wchar_t *GetUIDescription() const override { return L"Copy a file to certain location"; }
	virtual void Execute(WWKey eInput) const override
	{
		if (!std::filesystem::exists("debug"))
			std::filesystem::create_directory("debug");
		std::ifstream fileWithFilesToDump{"debug\\filesToCopy.txt"};
		if (fileWithFilesToDump.is_open())
		{
			std::string line;
			int n = 0;
			while (std::getline(fileWithFilesToDump, line))
			{
				CCFileClass file{line.c_str()};
				if (void *buffer = file.ReadWholeFile())
				{
					CCFileClass copyFile{(std::string("debug\\") + line).c_str()};
					copyFile.WriteBytes(buffer, file.GetFileSize());
					file.Close();
					copyFile.Close();
					n++;
				}
			}
			fileWithFilesToDump.close();
			MessageListClass::Instance->PrintMessage(std::format(L"Dumped {} files",n).c_str());
		}
	}
};

template <typename T>
void MakeCommand()
{
	T *command = GameCreate<T>();
	CommandClass::Array->AddItem(command);
};

DEFINE_HOOK(0x533066, CommandClassCallback_RegisterBlyat, 0x6)
{
	MakeCommand<NothingCommand>();
	MakeCommand<FileDumpCommand>();
	return 0;
}
