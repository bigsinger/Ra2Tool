#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>
#include <Unsorted.h>
#include "Utils.h"
#include "Config.h"

namespace {

constexpr const char* LOG_PREFIX = "[Ra2Tool] ";

void OutputRa2ToolLog(const char* msg) {
	char buffer[1200] = {};
	sprintf_s(buffer, sizeof(buffer), "%s%s", LOG_PREFIX, msg ? msg : "");
	OutputDebugStringA(buffer);
}

bool IsRa2ToolWindowClass(HWND hwnd) {
	char className[64] = {};
	GetClassNameA(hwnd, className, sizeof(className));
	return strcmp(className, "RA2ToolWindow") == 0
		|| strcmp(className, "RA2CustomToolbar") == 0
		|| strcmp(className, "RA2EnemyInfoPanel") == 0
		|| strcmp(className, "RA2TipWindow") == 0;
}

}

// 打印日志信息
void Utils::Log(DWORD error, const char* msg/* = NULL*/) {
	if (Config::isDebugMode()) {
		char buffer[1024] = {};
		sprintf_s(buffer, sizeof(buffer), "Error: %lu Message: %s", error, msg ? msg : "");
		OutputRa2ToolLog(buffer);
	}
}

// 打印日志信息
void Utils::Log(const char* msg) {
	if (Config::isDebugMode()) {
		OutputRa2ToolLog(msg);
	}
}

// 打印日志信息
void Utils::LogFormat(const char* pFormat, ...) {
	if (Config::isDebugMode()) {
		char buffer[1024] = {};
		va_list args;
		va_start(args, pFormat);
		vsprintf_s(buffer, pFormat, args);
		OutputRa2ToolLog(buffer);
		va_end(args);
	}
}

// 获取模块所在目录，最后带斜杠
void Utils::GetStartPath(HMODULE hModule, TCHAR* szBuffer, int nBufferCountSize/* = MAX_PATH*/) {
	GetModuleFileName(hModule, szBuffer, nBufferCountSize);
	_tcsrchr(szBuffer, '\\')[1] = 0;
}

HWND GetMainWindowForProcessId(DWORD targetPid) {
	__try {
		HWND gameHwnd = Game::hWnd;
		DWORD gamePid = 0;
		if (gameHwnd && IsWindow(gameHwnd)) {
			GetWindowThreadProcessId(gameHwnd, &gamePid);
			if (gamePid == targetPid) {
				return gameHwnd;
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}

	HWND targetHwnd = NULL;
    DWORD pid = 0;
    HWND hwnd = GetTopWindow(NULL);
    while (hwnd != NULL) {
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid == targetPid) {
            // 过滤子窗口，确保是顶层窗口（无 owner）+ 可见
            if (GetWindow(hwnd, GW_OWNER) == NULL && IsWindowVisible(hwnd) && !IsRa2ToolWindowClass(hwnd)) {
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
