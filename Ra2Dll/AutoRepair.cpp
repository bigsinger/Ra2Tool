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

// 开启自动修理功能
void AutoRepair() {
	RepairNextBuilding();
}
