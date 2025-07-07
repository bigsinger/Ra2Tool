// Ra2Dll.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "Utils.h"
#include "Ra2Dll.h"
#include "Config.h"
#include "Ra2Helper.h"
#include "AutoRepair.h"
#include "ToolWindow.h"

//#include <Utilities/Debug.h>
//#include <Utilities/Macro.h>
//#include "Ext/NetHack.h"


// 功能开启
void Open() {
	if (IsGaming()) {
		//{ // NetHack
		//	Patch::Apply_CALL(0x7B3D75, NetHack::SendTo);   // UDPInterfaceClass::Message_Handler
		//	Patch::Apply_CALL(0x7B3EEC, NetHack::RecvFrom); // UDPInterfaceClass::Message_Handler
		//}
		if (Config::isAutoOpenMap()) {
			Utils::Log("Auto Open Map!");
			OpenMap();
			OpenRadar();
		}
	} else {
		Utils::Log("Game not running!");
	}
}

RA2DLL_API int fnRa2Dll(void) {
	Open();
	return 0;
}
