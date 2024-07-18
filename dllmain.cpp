#include <Windows.h>
#include <utility>
#include <Dbghelp.h>
#include <TlHelp32.h>
#include <cstdio>
#include <winternl.h>
#include <Syringe.h>

#include <ASMMacros.h>
#include "Utilities/Debug.h"

bool DetachFromDebugger()
{
	auto GetDebuggerProcessId = [](DWORD dwSelfProcessId) -> DWORD
	{
		DWORD dwParentProcessId = -1;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(2, 0);
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hSnapshot, &pe32);
		do
		{
			if (pe32.th32ProcessID == dwSelfProcessId)
			{
				dwParentProcessId = pe32.th32ParentProcessID;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32));
		CloseHandle(hSnapshot);
		return dwParentProcessId;
	};

	HMODULE hModule = LoadLibrary("ntdll.dll");
	if (hModule != NULL)
	{
		auto const NtRemoveProcessDebug =
			(NTSTATUS(__stdcall*)(HANDLE, HANDLE))GetProcAddress(hModule, "NtRemoveProcessDebug");
		auto const NtSetInformationDebugObject =
			(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(hModule, "NtSetInformationDebugObject");
		auto const NtQueryInformationProcess =
			(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(hModule, "NtQueryInformationProcess");
		auto const NtClose =
			(NTSTATUS(__stdcall*)(HANDLE))GetProcAddress(hModule, "NtClose");

		HANDLE hDebug;
		HANDLE hCurrentProcess = GetCurrentProcess();
		NTSTATUS status = NtQueryInformationProcess(hCurrentProcess, 30, &hDebug, sizeof(HANDLE), 0);
		if (0 <= status)
		{
			ULONG killProcessOnExit = FALSE;
			status = NtSetInformationDebugObject(
				hDebug,
				1,
				&killProcessOnExit,
				sizeof(ULONG),
				NULL
			);
			if (0 <= status)
			{
				const auto pid = GetDebuggerProcessId(GetProcessId(hCurrentProcess));
				status = NtRemoveProcessDebug(hCurrentProcess, hDebug);
				if (0 <= status)
				{
					HANDLE hDbgProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
					if (INVALID_HANDLE_VALUE != hDbgProcess)
					{
						BOOL ret = TerminateProcess(hDbgProcess, EXIT_SUCCESS);
						CloseHandle(hDbgProcess);
						return ret;
					}
				}
			}
			NtClose(hDebug);
		}
		FreeLibrary(hModule);
	}

	return false;
}

#pragma pack(push, 1)
#pragma warning(push)
struct _LJMP
{
	byte opcode;
	DWORD pointer;

	_LJMP(DWORD offset, void* pointer) :
		opcode(0xE9),
		pointer((DWORD)pointer - offset - 5)
	{
		Apply(offset);
	};
private:
	void Apply(DWORD offset)
	{
		void* pAddress = (void*)offset;
		DWORD protect_flag;
		VirtualProtect(pAddress, sizeof(*this), PAGE_EXECUTE_READWRITE, &protect_flag);
		memcpy(pAddress, this, sizeof(*this));
		VirtualProtect(pAddress, sizeof(*this), protect_flag, NULL);
	}
};
#pragma warning(pop)
#pragma pack(pop)

void __declspec(naked) _ExeTerminate()
{
	// Call WinMain
	SET_REG32(EAX, 0x6BB9A0);
	CALL(EAX);
	PUSH_REG(EAX);

	__asm {call Console::Release};

	// Jump back
	POP_REG(EAX);
	SET_REG32(EBX, 0x7CD8EF);
	__asm {jmp ebx};
}

DEFINE_HOOK(0x7CD810, ExeRun_Phoboshit, 0x9)
{
#if I_FUCKING_DEBUG
	if (DetachFromDebugger())
	{
		MessageBoxW(NULL,
			L"You can now attach a debugger.\n\n"

			L"Press OK to continue YR execution.",
			L"Debugger Notice", MB_OK);
	}
	else
	{
		MessageBoxW(NULL,
			L"You can now attach a debugger.\n\n"

			L"To attach a debugger find the YR process in Process Hacker "
			L"/ Visual Studio processes window and detach debuggers from it, "
			L"then you can attach your own debugger. After this you should "
			L"terminate Syringe.exe because it won't automatically exit when YR is closed.\n\n"

			L"Press OK to continue YR execution.",
			L"Debugger Notice", MB_OK);
	}
#endif

#if I_BORROW_CONSOLE
	if (!Console::Create())
	{
		MessageBoxW(NULL,
			L"Failed to allocate the debug console!",
			L"Debug Console Notice", MB_OK);
	}

	_LJMP{ 0x7CD8EA ,_ExeTerminate };
#endif
    return 0;
}
