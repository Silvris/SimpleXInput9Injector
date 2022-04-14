// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <filesystem>
#include <winternl.h>
#include <TlHelp32.h>
#include <SDKDDKVer.h>
#include <fstream>
#include <vector>
#include <Windows.h>

#include "xinput.h"
#include "MemoryModule.h"
#include <Unknwn.h>
std::ofstream out("outLog.txt");

auto LoadDll(const char* path) {
    std::ifstream dll(path, std::ios::binary);
    std::vector<char> dllRead(std::istreambuf_iterator<char>(dll), {});

    size_t size = dllRead.size();
    char* allocated = (char*)malloc(size);
    out << allocated << std::endl;
    out << size << std::endl;
    if (allocated&&size) {
        memcpy(allocated, &dllRead[0], size);
        return MemoryLoadLibraryEx(allocated, size, MemoryDefaultAlloc, MemoryDefaultFree, MemoryDefaultLoadLibrary, MemoryDefaultGetProcAddress, MemoryDefaultFreeLibrary, nullptr);

    }
}

void Initialize() {
    out << "Start\n";
    char syspath[MAX_PATH];
    GetSystemDirectoryA(syspath, MAX_PATH);
    strcat_s(syspath, "\\xinput9_1_0.dll");
    hMod = LoadLibraryA(syspath);
    if (hMod > (HMODULE)31) {
        oXInputEnable = (tXInputEnable)GetProcAddress(hMod, "XInputEnable");
        oXInputGetState = (tXInputGetState)GetProcAddress(hMod, "XInputGetState");
        oXInputSetState = (tXInputSetState)GetProcAddress(hMod, "XInputSetState");
        oXInputGetKeystroke = (tXInputGetKeystroke)GetProcAddress(hMod, "XInputGetKeystroke");
        oXInputGetCapabilities = (tXInputGetCapabilities)GetProcAddress(hMod, "XInputGetCapabilities");
        oXInputGetDSoundAudioDeviceGuids = (tXInputGetDSoundAudioDeviceGuids)GetProcAddress(hMod, "XInputGetDSoundAudioDeviceGuids");
        oXInputGetBatteryInformation = (tXInputGetBatteryInformation)GetProcAddress(hMod, "XInputGetBatteryInformation");
        oXInputGetAudioDeviceIds = (tXInputGetAudioDeviceIds)GetProcAddress(hMod, "XInputGetAudioDeviceIds");
    }
    out << "XInputCreate\n" << syspath << "\n";
    //now load any plugins within our folder
    if (std::filesystem::exists("injectPlugins")) {
        for (auto& plugin : std::filesystem::directory_iterator("injectPlugins")) {
            std::string name = plugin.path().filename().string();
            if (plugin.path().filename().extension().string() == ".dll") {
                out << "Loading " << name << "\n";
                auto dll = LoadDll(plugin.path().string().c_str());
                if (!dll) out << "Failed to load " << name << "\n";
            }
        }
    }
    out.close();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Initialize();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

