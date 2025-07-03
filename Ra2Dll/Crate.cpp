#pragma once
#include <MapClass.h>
#include "Crate.h"
#include "Utils.h"

// 定义一个字符串数组，每个字符串对应一个枚举值
const char* powerupNames[] = {
    "Money",
    "Unit",
    "HealBase",
    "Cloak",
    "Explosion",
    "Napalm",
    "Squad",
    "Darkness",
    "Reveal",
    "Armor",
    "Speed",
    "Firepower",
    "ICBM",
    "Invulnerability",
    "Veteran",
    "IonStorm",
    "Gas",
    "Tiberium",
    "Pod"
};

const char* getCrateName(byte crateType) {
    Powerup powerupType = (Powerup)(crateType);
    if (powerupType >= Powerup::Money && powerupType <= Powerup::Pod) {
        return powerupNames[crateType];
    }
    return "Unknown";
}

// PlacePowerupCrate_56BEC0  
void ReadCrateType() {
	byte* p = (byte*)0x00ABDC50;
	Powerup powerup = (Powerup)(p[0x11E]);
    Utils::LogFormat("0x00ABDC50[0x11E] Type: %d Name: %s", powerup, getCrateName((byte)powerup));

	// 读取地图上的箱子数据
	MapClass&map = MapClass::Instance;
    for (int i = 0; i < 0x100; i++) {
        CellClass* cell = map.TryGetCellAt(map.Crates[i].Location);
        if (cell) {
            Utils::LogFormat("MapClass::Crates[%d] Type: %d Name: %s ", i, cell->OverlayData, getCrateName((byte)cell->OverlayData));
        } else {
            break;
        }
    }
}