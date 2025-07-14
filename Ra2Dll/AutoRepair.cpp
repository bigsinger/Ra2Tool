#include <windows.h>
#include <EventClass.h>
#include <HouseClass.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Helper.h"


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

// .text:004AC0D8
void RepairNextBuilding() {
	int count = 0;
	for (int i = 0; i < HouseClass::CurrentPlayer->Buildings.Count; i++) {
		BuildingClass* building = HouseClass::CurrentPlayer->Buildings.GetItem(i);
		if (!building->IsBeingRepaired && building->Health < building->GetType()->Strength) {
			//if (building->Health / building->GetType()->Strength < 0.7f) {
			EventClass repairEvent(0, 0);
			MakeEventClass(&repairEvent, HouseClass::CurrentPlayer->ArrayIndex, EventType::Repair,
				static_cast<int>(building->UniqueID), static_cast<int>(AbstractType::Abstract));
			EventClass::AddEvent(repairEvent);
			
			if ((++count) >= Config::getRepairCount()) {
				break;
			}
		}
	}
}

// 开启自动修理功能
void AutoRepair() {
	Utils::Log("Auto Repairing");
	RepairNextBuilding();
}
