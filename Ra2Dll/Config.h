#pragma once

const int UNINITED_FLAG = -1;

class Config {
private:
	// 配置文件路径
	static char _configFilePath[MAX_PATH];
public:
	static void SetConfigFilePath(const char* path) {
		strcpy_s(_configFilePath, path);
	}

private:
	// 是否自动打开全图
	static int _iAutoMapFlag;
public:
	static bool isAutoOpenMap() {
		if (_iAutoMapFlag == UNINITED_FLAG) {
			_iAutoMapFlag = ::GetPrivateProfileInt("main", "AutoOpenMap", TRUE, _configFilePath);
		}
		return _iAutoMapFlag > 0;
	}

private:
	// 是否打开RA2本身的日志
	static int _iRA2LogFlag;
public:
	static bool isOpenRA2Log() {
		if (_iRA2LogFlag == UNINITED_FLAG) {
			_iRA2LogFlag = ::GetPrivateProfileInt("main", "log", FALSE, _configFilePath);
		}
		return _iRA2LogFlag > 0;
	}


private:
	// 是否禁止隐身
	static int _iRA2DisableDisguise;
public:
	static bool isDisableDisguise() {
		if (_iRA2DisableDisguise == UNINITED_FLAG) {
			_iRA2DisableDisguise = ::GetPrivateProfileInt("main", "DisableDisguise", TRUE, _configFilePath);
		}
		return _iRA2DisableDisguise > 0;
	}

private:
	// 是否打开调试模式
	static int _iDebugFlag;
public:
	static int isDebugMode() {
		if (_iDebugFlag == UNINITED_FLAG) {
			_iDebugFlag = ::GetPrivateProfileInt("main", "debug", FALSE, _configFilePath);
		}
		return _iDebugFlag > 0;
	}

private:
	// 自动修理数量
	static int _iAutoRepairCount;
public:
	static int GetAutoRepairCount() {
		if (_iAutoRepairCount == UNINITED_FLAG) {
			_iAutoRepairCount = ::GetPrivateProfileInt("main", "AutoRepairCount", 99, _configFilePath);
		}
		return _iAutoRepairCount;
	}

private:
	// 获取箱子信息
	static int _iAutoShowCrateFlag;
public:
	static int isAutoShowCrate() {
		if (_iAutoShowCrateFlag == UNINITED_FLAG) {
			_iAutoShowCrateFlag = ::GetPrivateProfileInt("main", "crate", FALSE, _configFilePath);
		}
		return _iAutoShowCrateFlag > 0;
	}
};