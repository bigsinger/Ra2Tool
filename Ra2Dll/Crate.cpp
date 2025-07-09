#pragma once
#include <MapClass.h>
#include <CCINIClass.h>
#include <TacticalClass.h>
#include <OverlayTypeClass.h>
#include "Crate.h"
#include "Utils.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"
#include "TipWindow.h"



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

void ShowCrateLabel(HWND hWndParent, std::vector<HWND>& labels, int index, int timeLeft, bool visible, int posX, int posY, const wchar_t* szCrateName) {
    //Utils::LogFormat("ShowCrateLabel hwnd:%p %d  (%d %d)", labels[index], index, posX, posY);
    wchar_t szLabelText[60] = { 0 };
    wchar_t szArrow[4] = { 0 };

    // 不可见时需要校正位置和提示标签
    if (!visible) {
        if (posX < 0) {
            posX = 0;
            wcscpy_s(szArrow, _countof(szArrow), L"←");
        } else if (posX > (gameClientRect.right - CRATE_LABEL_WIDTH)) {
            posX = gameClientRect.right - CRATE_LABEL_WIDTH;
            wcscpy_s(szArrow, _countof(szArrow), L"→");
        }

        if (posY < 0) {
            posY = 0;
            wcscpy_s(szArrow, _countof(szArrow), L"↑");
        } else if (posY > (gameClientRect.bottom - CRATE_LABEL_HEIGHT)) {
            posY = gameClientRect.bottom - CRATE_LABEL_HEIGHT;
            wcscpy_s(szArrow, _countof(szArrow), L"↓");
        }
    }else{
    }

    swprintf(szLabelText, _countof(szLabelText), L"%s%s\n%.1f", szCrateName, szArrow, (timeLeft/(float)60)); // 帧数转换为秒数
    HWND hwnd = labels[index];
    if (hwnd) {
        SetWindowPos(hwnd, NULL, posX, posY, CRATE_LABEL_WIDTH, CRATE_LABEL_HEIGHT, SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowTextW(hwnd, szLabelText);
    } else {
        labels[index] = hwnd = createCrateLabel(hWndParent, szLabelText, posX, posY, CRATE_LABEL_WIDTH, CRATE_LABEL_HEIGHT);
    }
    ShowWindow(hwnd, SW_SHOW);
}

static int maxCrateIndex = 0; // 用于跟踪最大箱子索引
void ShowCrateInfo(HWND hwnd, std::vector<HWND>& labels) {
    //ShowCrateLabel(hwnd, labels, 0, 2000, false,  -80, 60, L"箱子"); return;
    // 先隐藏所有标签
    __try {
        for (int i = 0; i < maxCrateIndex; i++) {
            HWND h = labels[i];
            if (h) { ShowWindow(h, SW_HIDE); }
        }

        // 读取地图上的箱子数据
        MapClass& map = MapClass::Instance;
        for (int i = 0; i < 0x100; i++) {
            int timeLeft = map.Crates[i].CrateTimer.TimeLeft;
            if (timeLeft <= 0) { continue; }

            CellClass* cell = map.TryGetCellAt(map.Crates[i].Location);
            if (cell && cell->OverlayTypeIndex != -1) {
                maxCrateIndex++;
                auto [pos, visible] = TacticalClass::Instance->CoordsToClient(cell->GetCoords());
                //Utils::LogFormat("MapClass::Crates[%d] Location: (%d:%d) ScreenLocation: (%d:%d) visible: %d  CrateTimer.TimeLeft: %d", i, map.Crates[i].Location.X, map.Crates[i].Location.Y, pos.X, pos.Y, visible, map.Crates[i].CrateTimer.TimeLeft);
                Powerup crateType = Powerup::Money;
                OverlayTypeClass* overlay = OverlayTypeClass::Array[cell->OverlayTypeIndex];
                if (overlay && overlay->Crate) {
                    /*
                    在 Red Alert 2 / YRpp 中，crate 类型并不是始终可靠地储存在 OverlayData 中的，仅当当前格子确实被标记为 crate 时，
                    且 OverlayType 是支持 crate 的 overlay，OverlayData 才是有效的 crate 类型。
                    */
                    crateType = (Powerup)(cell->OverlayData);
                }

                const wchar_t* szCrateName = getCrateName(crateType);
                //Utils::LogFormat("MapClass::Crates[%d] Type: %d  TimeLeft: %d", i, crateType, timeLeft);

                // 显示箱子标签
                ShowCrateLabel(hwnd, labels, i, timeLeft, visible, pos.X, pos.Y, szCrateName);
            } else {
                //break;    // 箱子不是连续存放的，不能直接跳出循环。
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}
