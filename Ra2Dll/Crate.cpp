#pragma once
//#pragma pack(push, 4)
#include <OverlayTypeClass.h>
#include <MapClass.h>
//#pragma pack(pop)
#include "Crate.h"
#include "Utils.h"

// 定义一个字符串数组，每个字符串对应一个枚举值
const char* powerupNames[] = {
    "Money（金钱）",
    "Unit（单位）",
    "HealBase（基地治疗）",
    "Cloak（隐形）",
    "Explosion（爆炸）",
    "Napalm",
    "Squad（小队）",
    "Darkness（黑暗）",
    "Reveal（揭示）",
    "Armor（护甲）",
    "Speed（加速）",
    "Firepower（火力）",
    "ICBM（洲际弹道导弹）",
    "Invulnerability（无敌）",
    "Veteran（升级）",
    "IonStorm（离子风暴）",
    "Gas（气体）",
    "Tiberium",
    "Pod（舱体）"
};

const char* getCrateName(Powerup crateType) {
    if (crateType >= Powerup::Money && crateType <= Powerup::Pod) {
        return powerupNames[(int)crateType];
    }
    return "Unknown";
}

// PlacePowerupCrate_56BEC0  
void ReadCrateType() {
	byte* p = (byte*)0x00ABDC50;
	Powerup powerup = (Powerup)(p[0x11E]);
    Utils::LogFormat("0x00ABDC50[0x11E] Type: %d Name: %s", powerup, getCrateName(powerup));

	// 读取地图上的箱子数据
	MapClass&map = MapClass::Instance;
    for (int i = 0; i < 0x100; i++) {
        if (map.Crates[i].CrateTimer.TimeLeft <= 0) {
            continue;
        }

        CellClass* cell = map.TryGetCellAt(map.Crates[i].Location);
        Utils::LogFormat("MapClass::Crates[%d] Location: (%d:%d) CrateTimer.TimeLeft: %d", i, map.Crates[i].Location.X, map.Crates[i].Location.Y, map.Crates[i].CrateTimer.TimeLeft);

        if (cell) {
            Powerup crate_type = Powerup::Money; 
#if 0
            if (cell->OverlayTypeIndex != -1 && OverlayTypeClass::Array[cell->OverlayTypeIndex]->Crate) {
                crate_type = (Powerup)(cell->OverlayData);
            }
#else
            crate_type = (Powerup)(cell->OverlayData);
#endif
            Utils::LogFormat("MapClass::Crates[%d] Type: %d Name: %s ", i, crate_type, getCrateName(crate_type));
        } else {
            break;
        }
    }
}

//bool PlacePowerupCrate_new(MapClass*pThis, CellStruct cell, Powerup type) {
//    Utils::LogFormat("PlacePowerupCrate Type: %d, Name: %s, Location:(%d:%d)", (byte)type, getCrateName(type), cell.X, cell.Y);
//    return true;
//}
//
//void HookPlacePowerupCrate() {
//    //Patch::Apply_CALL(0x56BEC0, PlacePowerupCrate_new);
//}