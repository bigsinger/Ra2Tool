﻿#pragma once
#include <MapClass.h>
#include <TacticalClass.h>
#include "Utils.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"


// 无法获取箱子类型，箱子类型是在捡起的时候才会被设置的。
#if 0
// 定义一个字符串数组，每个字符串对应一个枚举值
const wchar_t* powerupNames[] = {
    L"Money（金钱）",
    L"Unit（单位）",
    L"HealBase（基地治疗）",
    L"Cloak（隐形）",
    L"Explosion（爆炸）",
    L"Napalm",
    L"Squad（小队）",
    L"Darkness（黑暗）",
    L"Reveal（揭示）",
    L"Armor（护甲）",
    L"Speed（加速）",
    L"Firepower（火力）",
    L"ICBM（洲际弹道导弹）",
    L"Invulnerability（无敌）",
    L"Veteran（升级）",
    L"IonStorm（离子风暴）",
    L"Gas（气体）",
    L"Tiberium",
    L"Pod（舱体）"
};

const wchar_t* getCrateName(Powerup crateType) {
    if (crateType >= Powerup::Money && crateType <= Powerup::Pod) {
        return powerupNames[(int)crateType];
    }
    return L"箱子";
}
#endif


void ShowCrateLabel(HDC hdc, bool visible, int posX, int posY, const wchar_t* szCrateName) {
    wchar_t szLabelText[32] = { L"箱子" };
    if (szCrateName) {
        wcscpy_s(szLabelText, _countof(szLabelText), szCrateName);
    }

    // 不可见时需要校正位置和提示标签
    if (!visible) {
        if (posX < 0) {
            posX = 0;
            wcscat_s(szLabelText, _countof(szLabelText), L"←");
        } else if (posX > (gameClientRect.right - CRATE_LABEL_WIDTH)) {
            posX = gameClientRect.right - CRATE_LABEL_WIDTH;
            wcscat_s(szLabelText, _countof(szLabelText), L"→");
        }

        if (posY < 0) {
            posY = 0;
            wcscat_s(szLabelText, _countof(szLabelText), L"↑");
        } else if (posY > (gameClientRect.bottom - CRATE_LABEL_HEIGHT)) {
            posY = gameClientRect.bottom - CRATE_LABEL_HEIGHT;
            wcscat_s(szLabelText, _countof(szLabelText), L"↓");
        }
    }

    //Utils::LogFormat("crate pos: (%d:%d)", posX, posY);
    TextOutW(hdc, posX, posY, szLabelText, wcslen(szLabelText));
}

void ShowCrateInfo(HDC hdc) {
#ifdef DEVDEBUG
    std::srand(static_cast<unsigned int>(std::time(nullptr)));  // 设置随机种子
    int x = std::rand() % 2001 - 1000;  // 范围[-1000, 1000]
    int y = std::rand() % 2001 - 1000;  // 范围[-1000, 1000]
    ShowCrateLabel(hdc, false,  x, y, NULL); return;
#endif

    __try {
        // 读取地图上的箱子数据
        MapClass& map = MapClass::Instance;
        for (int i = 0; i < CRATE_MAX_COUNT; i++) {
            int timeLeft = map.Crates[i].CrateTimer.TimeLeft;
            if (timeLeft <= 0) { continue; }

            CellClass* cell = map.TryGetCellAt(map.Crates[i].Location);
            if (cell && cell->OverlayTypeIndex != -1) {
                auto [pos, visible] = TacticalClass::Instance->CoordsToClient(cell->GetCoords());
                //Utils::LogFormat("MapClass::Crates[%d] Location: (%d:%d) ScreenLocation: (%d:%d) visible: %d  CrateTimer.TimeLeft: %d", i, map.Crates[i].Location.X, map.Crates[i].Location.Y, pos.X, pos.Y, visible, map.Crates[i].CrateTimer.TimeLeft);
#if 1
                // 这里不访问OverlayTypeClass::Array，以免游戏假死。
                ShowCrateLabel(hdc, visible, pos.X, pos.Y, NULL);
#else
                OverlayTypeClass* overlay = OverlayTypeClass::Array[cell->OverlayTypeIndex];
                if (overlay && overlay->Crate) {
                    /*
                    在 Red Alert 2 / YRpp 中，crate 类型并不是始终可靠地储存在 OverlayData 中的，仅当当前格子确实被标记为 crate 时，
                    且 OverlayType 是支持 crate 的 overlay，OverlayData 才是有效的 crate 类型。
                    */
                    Powerup crateType = (Powerup)(cell->OverlayData);
                    const wchar_t* szCrateName = getCrateName(crateType);
                    //Utils::LogFormat("MapClass::Crates[%d] Type: %d  TimeLeft: %d", i, crateType, timeLeft);
                    // 显示箱子标签
                    ShowCrateLabel(hdc, visible, pos.X, pos.Y, szCrateName);
                }
#endif // 1
            } else {
                //break;    // 箱子不是连续存放的，不能直接跳出循环。
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}
