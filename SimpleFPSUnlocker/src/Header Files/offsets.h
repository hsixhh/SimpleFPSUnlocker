#include <Windows.h>
#include <cstdint>
#include <cstdio>

namespace Utils
{
	/*Was told this method was shitty BUT it works so*/

	bool Compare(const BYTE* pData, const BYTE* bMask, const char* szMask)
	{
		for (; *szMask; ++szMask, ++pData, ++bMask)
			if (*szMask == 'x' && *pData != *bMask) return 0;
		return (*szMask) == NULL;
	}

	DWORD SignatureScan(const char* AoB, const char* Mask) {
		for (DWORD i = (DWORD)GetModuleHandle(0); i <= 0xF000000; ++i) {

			if (Compare((BYTE*)i, (BYTE*)AoB, Mask))
				return i;
		}
		return 0;
	}

	std::uintptr_t jmp;
	void open_console(const char* name)
	{
		const auto lib = LoadLibraryA("KERNEL32.dll");

		if (!lib)
			return;

		const auto free_console = reinterpret_cast<std::uintptr_t>(GetProcAddress(lib, "FreeConsole"));

		if (free_console)
		{
			jmp = free_console + 0x6;

			DWORD old{};

			VirtualProtect(reinterpret_cast<void*>(free_console), 0x6, PAGE_EXECUTE_READWRITE, &old);

			*reinterpret_cast<std::uintptr_t**>(free_console + 0x2) = &jmp;
			*reinterpret_cast<std::uint8_t*>(free_console + 0x6) = 0xC3;

			VirtualProtect(reinterpret_cast<void*>(free_console), 0x6, old, &old);
		}

		AllocConsole();

		FILE* file_stream;

		freopen_s(&file_stream, "CONIN$", "r", stdin);
		freopen_s(&file_stream, "CONOUT$", "w", stdout);
		freopen_s(&file_stream, "CONOUT$", "w", stderr);

		fclose(file_stream);

		SetConsoleTitleA(name);
		::SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
}

namespace offsets
{
	constexpr std::uintptr_t FrameDelay = 272;

	using get_task_scheduler_t = std::uintptr_t(*)();
	std::uintptr_t _sig = Utils::SignatureScan("\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x08\xE8\x00\x00\x00\x00\x8D\x0C\x24", "xxxxxxxxxx????xxx"); /*this change? not recently*/
	const auto GetTaskScheduler = reinterpret_cast<get_task_scheduler_t>(_sig + 14 + *reinterpret_cast<std::uint32_t*>(_sig + 10));
}

namespace Globals
{
	float FrameCap = 60.0f; /*60 is the default cap by roblox*/
	std::uintptr_t Scheduler; /*gonna set the scheduler for first time on injection*/
}
