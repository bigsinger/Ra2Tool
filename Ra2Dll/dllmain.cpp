// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "Ra2Dll.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        g_thisModule = hModule;
        Install(hModule);
        break;
    case DLL_PROCESS_DETACH:
        Uninstall();
        break;
    }
    return TRUE;
}
