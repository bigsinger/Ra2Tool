#pragma once
#include <list>
#include <string>

const int UNINITED_FLAG = -1;

class Config {
private:
	// 配置文件路径
	inline static char _configFilePath[MAX_PATH] = {};
public:
	static void setConfigFilePath(const char* path) {
		strcpy_s(_configFilePath, path);
	}

private:
	// 是否打开调试模式
	inline static int _iDebugFlag = UNINITED_FLAG;
public:
	static int isDebugMode() {
		if (_iDebugFlag == UNINITED_FLAG) {
			_iDebugFlag = ::GetPrivateProfileInt("main", "debug", TRUE, _configFilePath);
		}
		return _iDebugFlag > 0;
	}

private:
	// 是否打开RA2本身的日志
	inline static int _iRA2LogFlag = UNINITED_FLAG;
public:
	static bool isOpenRA2Log() {
		if (_iRA2LogFlag == UNINITED_FLAG) {
			_iRA2LogFlag = ::GetPrivateProfileInt("main", "log", FALSE, _configFilePath);
		}
		return _iRA2LogFlag > 0;
	}

private:
	// 是否自动打开全图
	inline static int _iAutoMapFlag = UNINITED_FLAG;
public:
	static bool isAutoOpenMap() {
		if (_iAutoMapFlag == UNINITED_FLAG) {
			_iAutoMapFlag = ::GetPrivateProfileInt("main", "AutoOpenMap", TRUE, _configFilePath);
		}
		return _iAutoMapFlag > 0;
	}

private:
	// 是否自动打开雷达
	inline static int _iAutoRadarFlag = UNINITED_FLAG;
public:
	static bool isAutoOpenRadar() {
		if (_iAutoRadarFlag == UNINITED_FLAG) {
			_iAutoRadarFlag = ::GetPrivateProfileInt("main", "AutoOpenRadar", TRUE, _configFilePath);
		}
		return _iAutoRadarFlag > 0;
	}

private:
	// 是否禁止隐身
	inline static int _iRA2DisableDisguise = UNINITED_FLAG;
public:
	static bool isDisableDisguise() {
		if (_iRA2DisableDisguise == UNINITED_FLAG) {
			_iRA2DisableDisguise = ::GetPrivateProfileInt("main", "DisableDisguise", TRUE, _configFilePath);
		}
		return _iRA2DisableDisguise > 0;
	}

private:
	// 自动修理时间间隔（单位：秒）
	inline static int _iAutoRepairTime = UNINITED_FLAG;
public:
	static int getAutoRepairTime() {
		if (_iAutoRepairTime == UNINITED_FLAG) {
			_iAutoRepairTime = ::GetPrivateProfileInt("main", "AutoRepairTime", 0, _configFilePath);
		}
		return _iAutoRepairTime;
	}

private:
	// 修理数量
	inline static int _iRepairCount = UNINITED_FLAG;
public:
	static int getRepairCount() {
		if (_iRepairCount == UNINITED_FLAG) {
			_iRepairCount = ::GetPrivateProfileInt("main", "RepairCount", 99, _configFilePath);
		}
		return _iRepairCount;
	}

private:
	// 获取箱子信息
	inline static int _iAutoShowCrateFlag = UNINITED_FLAG;
public:
	static int isAutoShowCrate() {
		if (_iAutoShowCrateFlag == UNINITED_FLAG) {
			_iAutoShowCrateFlag = ::GetPrivateProfileInt("main", "crate", FALSE, _configFilePath);
		}
		return _iAutoShowCrateFlag > 0;
	}

private:
	// 心灵探测
	inline static int _iAutoOpenPsiSensorFlag = UNINITED_FLAG;
public:
	static int isAutoOpenPsiSensor() {
		if (_iAutoOpenPsiSensorFlag == UNINITED_FLAG) {
			_iAutoOpenPsiSensorFlag = ::GetPrivateProfileInt("main", "xinling", FALSE, _configFilePath);
		}
		return _iAutoOpenPsiSensorFlag > 0;
	}

private:
	// 法国巨炮辅助开关
	inline static int _iGrandCannonAssistFlag = UNINITED_FLAG;
public:
	static bool isGrandCannonAssistEnabled() {
		if (_iGrandCannonAssistFlag == UNINITED_FLAG) {
			_iGrandCannonAssistFlag = ::GetPrivateProfileInt("grandcannon", "Enable", TRUE, _configFilePath);
		}
		return _iGrandCannonAssistFlag > 0;
	}

private:
	// 巨炮自动转向开关
	inline static int _iGrandCannonAutoTurnFlag = UNINITED_FLAG;
public:
	static bool isGrandCannonAutoTurnEnabled() {
		if (_iGrandCannonAutoTurnFlag == UNINITED_FLAG) {
			_iGrandCannonAutoTurnFlag = ::GetPrivateProfileInt("grandcannon", "AutoTurn", TRUE, _configFilePath);
		}
		return _iGrandCannonAutoTurnFlag > 0;
	}

	static void setGrandCannonAutoTurnEnabled(bool enabled) {
		_iGrandCannonAutoTurnFlag = enabled ? TRUE : FALSE;
	}

private:
	// 巨炮自动转向角度，默认0表示不转向
	inline static int _iGrandCannonTurnAngle = UNINITED_FLAG;
public:
	static int getGrandCannonTurnAngle() {
		if (_iGrandCannonTurnAngle == UNINITED_FLAG) {
			_iGrandCannonTurnAngle = ::GetPrivateProfileInt("grandcannon", "TurnAngle", 0, _configFilePath);
		}
		return _iGrandCannonTurnAngle;
	}

private:
	// 巨炮建筑类型编号，默认GTGCAN=64
	inline static int _iGrandCannonBuildingType = UNINITED_FLAG;
public:
	static int getGrandCannonBuildingType() {
		if (_iGrandCannonBuildingType == UNINITED_FLAG) {
			_iGrandCannonBuildingType = ::GetPrivateProfileInt("grandcannon", "BuildingType", 64, _configFilePath);
		}
		return _iGrandCannonBuildingType;
	}

private:
	// 巨炮辅助扫描间隔，单位毫秒
	inline static int _iGrandCannonScanInterval = UNINITED_FLAG;
public:
	static int getGrandCannonScanInterval() {
		if (_iGrandCannonScanInterval == UNINITED_FLAG) {
			_iGrandCannonScanInterval = ::GetPrivateProfileInt("grandcannon", "ScanInterval", 200, _configFilePath);
		}
		return _iGrandCannonScanInterval > 0 ? _iGrandCannonScanInterval : 200;
	}

private:
	// 是否显示敌方玩家信息
	inline static int _iShowEnemyInfoFlag = UNINITED_FLAG;
public:
	static bool isShowEnemyInfo() {
		if (_iShowEnemyInfoFlag == UNINITED_FLAG) {
			_iShowEnemyInfoFlag = ::GetPrivateProfileInt("ui", "ShowEnemyInfo", TRUE, _configFilePath);
		}
		return _iShowEnemyInfoFlag > 0;
	}

	static void setShowEnemyInfo(bool enabled) {
		_iShowEnemyInfoFlag = enabled ? TRUE : FALSE;
	}

private:
	// 是否显示底部自定义工具栏
	inline static int _iCustomToolbarFlag = UNINITED_FLAG;
public:
	static bool isCustomToolbarEnabled() {
		if (_iCustomToolbarFlag == UNINITED_FLAG) {
			_iCustomToolbarFlag = ::GetPrivateProfileInt("ui", "CustomToolbar", TRUE, _configFilePath);
		}
		return _iCustomToolbarFlag > 0;
	}

private:
	// 开启高科技是否使用自定义的配置
	inline static int _iOpenTechUsingCustom = UNINITED_FLAG;
public:
	static int isOpenTechUsingCustom() {
		if (_iOpenTechUsingCustom == UNINITED_FLAG) {
			_iOpenTechUsingCustom = ::GetPrivateProfileInt("main", "CustomTech", FALSE, _configFilePath);
		}
		return _iOpenTechUsingCustom > 0;
	}

private:
	inline static std::string strBuildingTypeTech = "-";
	inline static std::string strInfantryTypeTech = "-";
	inline static std::string strUnitTypeTech = "-";

	inline static std::list<int> vtBuildingTypeTech;
	inline static std::list<int> vtInfantryTypeTech;
	inline static std::list<int> vtUnitTypeTech;

public:
		static std::list<int>& getBuildingTypeTechList() {
			if (strBuildingTypeTech == "-") {
				char buf[1024] = { 0 };
				::GetPrivateProfileStringA("tech", "7", "", buf, sizeof(buf), _configFilePath);
				Utils::split(buf, " ", vtBuildingTypeTech);
			}
			return vtBuildingTypeTech;
		}

		static std::list<int>& getInfantryTypeTechList() {
			if (strInfantryTypeTech == "-") {
				char buf[1024] = { 0 };
				::GetPrivateProfileStringA("tech", "16", "", buf, sizeof(buf), _configFilePath);
				Utils::split(buf, " ", vtInfantryTypeTech);
			}
			return vtInfantryTypeTech;
		}

		static std::list<int>& getUnitTypeTechList() {
			if (strUnitTypeTech == "-") {
				char buf[1024] = { 0 };
				::GetPrivateProfileStringA("tech", "40", "", buf, sizeof(buf), _configFilePath);
				Utils::split(buf, " ", vtUnitTypeTech);
			}
			return vtUnitTypeTech;
		}
};
