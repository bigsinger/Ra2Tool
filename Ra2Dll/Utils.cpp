#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include "Utils.h"
#include "Config.h"


// 打印日志信息
void Utils::Log(DWORD error, const char* msg/* = NULL*/) {
	if (Config::isDebugMode()) {
		char buffer[1024] = {};
		sprintf_s(buffer, sizeof(buffer), "Error: %lu Message: %s", error, msg);
		OutputDebugStringA(buffer);
	}
}

// 打印日志信息
void Utils::Log(const char* msg) {
	if (Config::isDebugMode()) {
		OutputDebugStringA(msg);
	}
}

// 打印日志信息
void Utils::LogFormat(const char* pFormat, ...) {
	if (Config::isDebugMode()) {
		char buffer[1024] = {};
		va_list args;
		va_start(args, pFormat);
		vsprintf_s(buffer, pFormat, args);
		OutputDebugStringA(buffer);
		va_end(args);
	}
}

// 获取模块所在目录，最后带斜杠
void Utils::GetStartPath(HMODULE hModule, TCHAR* szBuffer, int nBufferCountSize/* = MAX_PATH*/) {
	GetModuleFileName(hModule, szBuffer, nBufferCountSize);
	_tcsrchr(szBuffer, '\\')[1] = 0;
}

HWND GetMainWindowForProcessId(DWORD targetPid) {
	HWND targetHwnd = NULL;
    DWORD pid = 0;
    HWND hwnd = GetTopWindow(NULL);
    while (hwnd != NULL) {
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid == targetPid) {
            // 过滤子窗口，确保是顶层窗口（无 owner）+ 可见
            if (GetWindow(hwnd, GW_OWNER) == NULL && IsWindowVisible(hwnd)) {
				targetHwnd = hwnd;
				break;
            }
        }
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }

    return targetHwnd;
}

size_t Utils::split(const std::string& strSrc, const std::string& strSep, std::list<int>& vtInt) {
	vtInt.clear();
	char* pContext = NULL;
	char* pToken = strtok_s((char*)strSrc.c_str(), strSep.c_str(), &pContext);	//_tcstok_s
	while (pToken != NULL) {
		int id = std::atoi(pToken);
		vtInt.push_back(id);
		pToken = strtok_s(NULL, strSep.c_str(), &pContext);
	}

	return vtInt.size();
}