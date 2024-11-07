// Ra2Dll.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "Ra2Dll.h"
#include "Ra2Helper.h"

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include "Ext/NetHack.h"


// 功能开启
void Open() {
	if (IsGameRunning()) {
		//{ // NetHack
		//	Patch::Apply_CALL(0x7B3D75, NetHack::SendTo);   // UDPInterfaceClass::Message_Handler
		//	Patch::Apply_CALL(0x7B3EEC, NetHack::RecvFrom); // UDPInterfaceClass::Message_Handler
		//}
		OpenMap();
		OpenRadar();
		OpenLog();
		//trainer.SetBoxAllMoney();
	} else {
		::Beep(523, 400);	// do
	}
}


RA2DLL_API int fnRa2Dll(void) {
	Open();
	return 0;
}
