#include "windows.h"
#include <stdio.h>
#include "Ra2Helper.h"
#include <EventClass.h>
#include <HouseClass.h>

// 打印错误信息
void log(DWORD error, const char* tag/* = NULL*/) {
	char buffer[1024] = {};
	sprintf_s(buffer, sizeof(buffer), "Tag: %s, Error Code: %lu", tag, error);
	OutputDebugStringA(buffer);
}

/////////////////////////////////////////
// 全局变量
static HANDLE hThreadAutoRepair = NULL;
static HANDLE hStopEventAutoRepair = NULL;
static BOOL isAutoRepairOpen = FALSE;
/////////////////////////////////////////


#pragma pack(1)

// YRpp 函数重载有问题，这里再写一遍
void MakeEventClass(EventClass* eventClass, int houseIndex, EventType eventType, int id, int rtti) {
	int typeId = static_cast<int>(eventType);
	__asm {
		pushad
		push rtti
		push id
		push typeId
		push houseIndex
		mov ecx, eventClass
		mov eax, 0x004C65E0
		call eax
		popad
	}
}

#pragma pack()

void RepairNextBuilding() {
	log(1, "1111");
	for (int i = 0; i < HouseClass::CurrentPlayer->Buildings.Count; i++) {
		BuildingClass* building = HouseClass::CurrentPlayer->Buildings.GetItem(i);
		if (building->Health < building->GetType()->Strength) {
			//if (building->Health / building->GetType()->Strength < 0.7f) {
			// .text:004AC0D8
			log(1, "2222");
			EventClass repairEvent(0, 0);
			MakeEventClass(&repairEvent, HouseClass::CurrentPlayer->ArrayIndex, EventType::Repair,
				static_cast<int>(building->UniqueID), static_cast<int>(AbstractType::Abstract));
			EventClass::AddEvent(repairEvent);
			// 每次只修 1 个
			break;
		}
	}
}


// 线程函数
DWORD WINAPI ThreadRepairBuilding(LPVOID lpParam) {
	while (1) {
		// 等待1秒，或直到收到停止信号
		DWORD waitResult = WaitForSingleObject(hStopEventAutoRepair, 1000);
		if (waitResult == WAIT_OBJECT_0) {
			break; // 收到停止事件，退出线程
		}

		RepairNextBuilding();
	}
	return 0;
}

// 开启自动修理功能
void OpenAutoRepair() {
	RepairNextBuilding();
	//if (!isAutoRepairOpen) {
	//	hStopEventAutoRepair = CreateEvent(NULL, TRUE, FALSE, NULL); // 手动重置，初始为非信号
	//	if (hStopEventAutoRepair) {
	//		hThreadAutoRepair = CreateThread(NULL, 0, ThreadRepairBuilding, NULL, 0, NULL);
	//		if (hThreadAutoRepair == NULL) {
	//			fprintf(stderr, "创建线程失败！\n");
	//			CloseHandle(hStopEventAutoRepair);
	//			hStopEventAutoRepair = NULL;
	//		} else {
	//			isAutoRepairOpen = TRUE;
	//			printf("线程已启动。\n");
	//		}
	//	}
	//} else {
	//	printf("线程已经在运行，无需重复启动。\n");
	//}
}

// 关闭自动修理功能
void CloseAutoRepair() {
	//if (isAutoRepairOpen) {
	//	SetEvent(hStopEventAutoRepair); // 通知线程停止

	//	WaitForSingleObject(hThreadAutoRepair, INFINITE);
	//	CloseHandle(hThreadAutoRepair);
	//	CloseHandle(hStopEventAutoRepair);

	//	hThreadAutoRepair = NULL;
	//	hStopEventAutoRepair = NULL;
	//	isAutoRepairOpen = FALSE;
	//	printf("线程已优雅退出。\n");
	//}
}
/////////////////////////////////////////