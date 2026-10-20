#pragma once

#include <string_view>
#include <string>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WINAPI
#define WINAPI __stdcall
#endif

extern "C"
{
    typedef void *HANDLE;
    typedef void *HMODULE;
    typedef const wchar_t *PCWSTR;
    typedef long HRESULT;

    __declspec(dllimport) HMODULE WINAPI GetModuleHandleW(const wchar_t *lpModuleName);
    __declspec(dllimport) void *WINAPI GetProcAddress(HMODULE hModule, const char *lpProcName);
    __declspec(dllimport) HANDLE WINAPI GetCurrentThread(void);
}
#elif defined(__vita__)
#include <psp2/kernel/cpu.h>
#include <psp2/kernel/threadmgr.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#elif defined(__APPLE__) || defined(__linux__)
#include <pthread.h>
#if defined(__APPLE__)
#include <pthread/qos.h>
#endif
#endif

namespace ThreadNaming
{
    inline void SetCurrentThreadName(std::string_view name)
    {
#if defined(_WIN32) 
        using SetThreadDescriptionFn = HRESULT(WINAPI*)(HANDLE, PCWSTR);

        HMODULE kernel32 = ::GetModuleHandleW(L"Kernel32.dll");
        auto setThreadDescription =
            reinterpret_cast<SetThreadDescriptionFn>(::GetProcAddress(kernel32, "SetThreadDescription"));

        if (setThreadDescription)
        {
            std::wstring wname(name.begin(), name.end());
            setThreadDescription(::GetCurrentThread(), wname.c_str());
        }

#elif defined(__APPLE__)
        pthread_setname_np(name.data());
        // The emulation threads (game, VU1, GIF, GS) are the game itself: keep
        // them ahead of background work (builds), which otherwise slows the
        // frame-paced loading screens to a crawl.
        pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);

#elif defined(__linux__)
        pthread_setname_np(pthread_self(), name.data());
#elif defined(__vita__)
        // Where the emulation threads run: logged, and with PS2X_PIN_THREADS=1
        // given a core each (the game thread one of its own, the native GL
        // thread another, the workers the third).
        const SceUID id = sceKernelGetThreadId();
        SceKernelThreadInfo info;
        std::memset(&info, 0, sizeof(info));
        info.size = sizeof(info);
        sceKernelGetThreadInfo(id, &info);
        int mask = 0;
        if (const char *pin = std::getenv("PS2X_PIN_THREADS"); pin && std::atoi(pin) != 0)
        {
            if (name == "GameThread")
                mask = SCE_KERNEL_CPU_MASK_USER_0;
            else if (name == "NativeGL")
                mask = SCE_KERNEL_CPU_MASK_USER_1;
            else
                mask = SCE_KERNEL_CPU_MASK_USER_2;
            sceKernelChangeThreadCpuAffinityMask(id, mask);
        }
        std::fprintf(stderr, "[thread] %.*s: affinity %x (initial %x), cpu %d, priority %d%s\n", static_cast<int>(name.size()), name.data(),
                     info.currentCpuAffinityMask, info.initCpuAffinityMask, info.currentCpuId, info.currentPriority, mask ? ", pinned" : "");
#endif
    }
}
