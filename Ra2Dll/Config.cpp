#include "windows.h"
#include "Config.h"


char Config::_configFilePath[MAX_PATH] = {};
int Config::_iAutoMapFlag = UNINITED_FLAG;
int Config::_iRA2LogFlag = UNINITED_FLAG;
int Config::_iDebugFlag = UNINITED_FLAG;
int Config::_iAutoRepairCount = UNINITED_FLAG;