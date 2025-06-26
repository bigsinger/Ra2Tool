#pragma once

class Utils {
public:
	// 打印日志信息，使用DebugView查看
	static void Log(DWORD error, const char* msg = NULL);
	static void Log(const char* msg);
	static void LogFormat(const char* pFormat, ...);

	// 获取模块所在目录，最后带斜杠
	static void GetStartPath(HMODULE hModule, TCHAR* szBuffer, int nBufferCountSize = MAX_PATH);
};