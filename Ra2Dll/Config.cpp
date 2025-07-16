#include "windows.h"
#include "Config.h"


char Config::_configFilePath[MAX_PATH] = {};
int Config::_iAutoMapFlag = UNINITED_FLAG;
int Config::_iAutoRadarFlag = UNINITED_FLAG;
int Config::_iRA2LogFlag = UNINITED_FLAG;
int Config::_iDebugFlag = UNINITED_FLAG;
int Config::_iAutoRepairTime = UNINITED_FLAG;
int Config::_iRepairCount = UNINITED_FLAG; 
int Config::_iRA2DisableDisguise = UNINITED_FLAG;
int Config::_iAutoShowCrateFlag = UNINITED_FLAG;
int Config::_iAutoOpenPsiSensorFlag = UNINITED_FLAG;
