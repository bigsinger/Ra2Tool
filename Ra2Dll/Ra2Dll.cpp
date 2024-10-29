// Ra2Dll.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "Ra2Dll.h"
#include "Ra2Helper.h"


// 功能开启
void Open() {
	if (IsGameRunning()) {
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
