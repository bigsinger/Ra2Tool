#include "windows.h"
#include <stdio.h>
#include <tchar.h>
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
