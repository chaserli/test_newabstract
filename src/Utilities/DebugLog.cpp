#include "Debug.h"
#include <cstdio>
#include <Helpers/Macro.h>
#include <ASMMacros.h>

void Debug::Log(const char* format,...)
{
    JMP_STD(0x4068E0);
}

std::ofstream Debug::MyLogFile{ "debug\\mylog.log" };

void Debug::LogToNewFile(const char* format,...)
{
	va_list args;
	va_start(args, format);
	Console::WriteFormat(format,args);
	if (MyLogFile.is_open())
		MyLogFile << StringBuffer;
	va_end(args);
}

char Debug::StringBuffer[0x1000];
static DWORD _Real_Debug_Log = 0x4A4AF9;
void __declspec(naked) _Fake_Debug_Log()
{
	__asm { mov ecx, [esp + 0x4] }
	__asm { lea edx, [esp + 0x8] }
	__asm { call Console::WriteWithVArgs }
	// __asm { mov edx, 0}

	// goto original bytes
	__asm { mov eax, _Real_Debug_Log }
	__asm { jmp eax }
}

Console::ConsoleTextAttribute Console::TextAttribute;
HANDLE Console::ConsoleHandle;

bool Console::Create()
{
	if (FALSE == AllocConsole())
		return false;

	ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (NULL == ConsoleHandle)
		return false;

	SetConsoleTitle("Console from Phobos Blyat");

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(ConsoleHandle, &csbi);
	TextAttribute.AsWord = csbi.wAttributes;

	PatchLog(0x4A4AC0, _Fake_Debug_Log, &_Real_Debug_Log);
	PatchLog(0x4068E0, _Fake_Debug_Log, nullptr);

	return true;
}

void Console::Release()
{
	if (NULL != ConsoleHandle)
		FreeConsole();
}

void Console::SetForeColor(ConsoleColor color)
{
	if (NULL == ConsoleHandle)
		return;

	if (TextAttribute.Foreground == color)
		return;

	TextAttribute.Foreground = color;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::SetBackColor(ConsoleColor color)
{
	if (NULL == ConsoleHandle)
		return;

	if (TextAttribute.Background == color)
		return;

	TextAttribute.Background = color;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::EnableUnderscore(bool enable)
{
	if (NULL == ConsoleHandle)
		return;

	if (TextAttribute.Underscore == enable)
		return;

	TextAttribute.Underscore = enable;
	SetConsoleTextAttribute(ConsoleHandle, TextAttribute.AsWord);
}

void Console::Write(const char* str, int len)
{
	if (NULL != ConsoleHandle)
		WriteConsole(ConsoleHandle, str, len, nullptr, nullptr);
}

void Console::WriteLine(const char* str, int len)
{
	Write(str, len);
	Write("\n");
}

void __fastcall Console::WriteWithVArgs(const char* pFormat, va_list args)
{
	vsprintf_s(Debug::StringBuffer, pFormat, args);
	Write(Debug::StringBuffer, strlen(Debug::StringBuffer));
}

void Console::WriteFormat(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	WriteWithVArgs(pFormat, args);
	va_end(args);
}

void Console::PatchLog(DWORD dwAddr, void* fakeFunc, DWORD* pdwRealFunc)
{
#pragma pack(push, 1)
	struct JMP_STRUCT
	{
		byte opcode;
		DWORD offset;
	} *pInst;
#pragma pack(pop)

	DWORD dwOldFlag;
	VirtualProtect((LPVOID)dwAddr, 5, PAGE_EXECUTE_READWRITE, &dwOldFlag);

	pInst = (JMP_STRUCT*)dwAddr;

	if (pdwRealFunc && pInst->opcode == 0xE9) // If this function is hooked by Ares
		*pdwRealFunc = pInst->offset + dwAddr + 5;

	pInst->offset = reinterpret_cast<DWORD>(fakeFunc) - dwAddr - 5;

	VirtualProtect((LPVOID)dwAddr, 5, dwOldFlag, NULL);
}
