#include <Windows.h>
#include <cstdint>
#include <thread>
#include <string>

#include "../Header Files/imgui-overlay.h"

auto m_main() -> void
{
    Utils::open_console({"SFPSU - by nicholas"});

    Globals::Scheduler = offsets::GetTaskScheduler();

    std::thread([] {
        while (true)
            *reinterpret_cast<double*>(Globals::Scheduler + offsets::FrameDelay) = 1.0 / Globals::FrameCap;
        }).detach();
        
    Initialize();
}

auto __stdcall DllMain(void*, std::uint32_t call_reason, void*) -> bool
{
    if (call_reason != 1)
        return false;

    std::thread(m_main).detach(); // uh oh!

    return true;
}