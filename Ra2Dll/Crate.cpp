#pragma once
#include <MapClass.h>
#include <CCINIClass.h>
#include <TacticalClass.h>
#include <OverlayTypeClass.h>
#include "Crate.h"
#include "Utils.h"

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
    return L"Unknown";
}

void CreateCrateLabel(HWND hWndParent, std::vector<HWND>& labels, int index, int posX, int posY) {
    HWND label = CreateWindowExW(
        WS_EX_TRANSPARENT | WS_EX_LAYERED,
        L"STATIC",
        NULL,
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        posX, posY, 100, 20,
        hWndParent,
        NULL,
        NULL,
        NULL
    );
    labels[index] = label;
	ShowWindow(label, SW_SHOW);
}

void ShowCrateLabel(HWND hWndParent, std::vector<HWND>& labels, int index, int posX, int posY, const wchar_t* szCrateName) {
    Utils::LogFormat("ShowCrateLabel %d  (%d %d)", index, posX, posY);
    if (labels[index]) {
        SetWindowPos(labels[index], NULL, posX, posY, 100, 20, SWP_NOZORDER | SWP_NOACTIVATE);
    } else {
        CreateCrateLabel(hWndParent, labels, index, posX, posY);
    }
	SetWindowTextW(labels[index], szCrateName);
}

void ShowCrateInfo(HWND hwnd, std::vector<HWND>& labels) {
	byte* p = (byte*)0x00ABDC50;
	Powerup powerup = (Powerup)(p[0x11E]);
    Utils::LogFormat("0x00ABDC50[0x11E] Type: %d Name: %s  offset: %d", powerup, getCrateName(powerup), offsetof(CellClass, OverlayData));

    //DrawCrateText(L"Test", hdc, 800, 600);

	// 读取地图上的箱子数据
	MapClass&map = MapClass::Instance;
    for (int i = 0; i < 0x100; i++) {
        if (map.Crates[i].CrateTimer.TimeLeft <= 0) {
            continue;
        }

        CellClass*cell = map.TryGetCellAt(map.Crates[i].Location);
        if (cell && cell->OverlayTypeIndex != -1) {
            auto [pos, visible] = TacticalClass::Instance->CoordsToClient(cell->GetCoords());
            Utils::LogFormat("MapClass::Crates[%d] Location: (%d:%d) ScreenLocation: (%d:%d) visible: %d  CrateTimer.TimeLeft: %d", i, map.Crates[i].Location.X, map.Crates[i].Location.Y, pos.X, pos.Y, visible, map.Crates[i].CrateTimer.TimeLeft);

            Powerup crate_type = Powerup::Money; 
            OverlayTypeClass* overlay = OverlayTypeClass::Array[cell->OverlayTypeIndex];
            if (overlay && overlay->Crate) {
                /*
                在 Red Alert 2 / YRpp 中，crate 类型并不是始终可靠地储存在 OverlayData 中的，仅当当前格子确实被标记为 crate 时，
                且 OverlayType 是支持 crate 的 overlay，OverlayData 才是有效的 crate 类型。
                */
                crate_type = (Powerup)(cell->OverlayData);
            }

			const wchar_t* szCrateName = getCrateName(crate_type);
            Utils::LogFormat("MapClass::Crates[%d] Type: %d", i, crate_type);

			// 显示箱子标签
            ShowCrateLabel(hwnd, labels, i, pos.X, pos.Y, szCrateName);
        } else {
			//break;    // 箱子不是连续存放的，不能直接跳出循环。
        }
    }
}
