#pragma once
#include <MapClass.h>
#include "Crate.h"
#include "Utils.h"

// 定义一个字符串数组，每个字符串对应一个枚举值
const char* powerupNames[] = {
    "Money（金钱）",
    "Unit（单位）",
    "HealBase（基地治疗）",
    "Cloak（隐形）",
    "Explosion（爆炸）",
    "Napalm（凝固汽油弹）",
    "Squad（小队）",
    "Darkness（黑暗）",
    "Reveal（揭示）",
    "Armor（护甲）",
    "Speed（速度）",
    "Firepower（火力）",
    "ICBM（洲际弹道导弹）",
    "Invulnerability（无敌）",
    "Veteran（老兵）",
    "IonStorm（离子风暴）",
    "Gas（气体）",
    "Tiberium（泰伯利亚矿）",
    "Pod（舱体）"
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