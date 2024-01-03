// Ra2Dll.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "Ra2Dll.h"
#include "..\RATrainer\trainerfunctions.h"


// 这是导出变量的一个示例
RA2DLL_API int nRa2Dll=0;

// 这是导出函数的一个示例。
RA2DLL_API int fnRa2Dll(void) {
	openAllMap();
    return 0;
}

// 这是已导出类的构造函数。
CRa2Dll::CRa2Dll()
{
    return;
}

//地图全开
void openAllMap() {
	::Beep(523, 400);	// do
	TrainerFunctions trainer;		// 修改器功能全局对象
	trainer.Map_Assemble();
}