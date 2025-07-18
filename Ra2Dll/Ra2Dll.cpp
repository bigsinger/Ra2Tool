// Ra2Dll.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "Utils.h"
#include "Ra2Dll.h"
#include "Config.h"
#include "Ra2Helper.h"

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
		Utils::Log("game running!");

		if (Config::isAutoOpenMap()) {
			Utils::Log("auto open map!");
			OpenMap();
		}

		if (Config::isAutoOpenRadar()) {
			Utils::Log("auto open radar!");
			OpenRadar();
		}

		if (Config::isDisableDisguise()) {
			DisableDisguise();
		}
	} else {
		Utils::Log("game not running!");
	}
}

RA2DLL_API int __stdcall fnRa2Dll(void) {
	Open();
	return 0;
}
