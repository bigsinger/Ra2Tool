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

Point2D GetScreenLocation(CellClass& cell) {
    return TacticalClass::Instance->CoordsToClient(cell.GetCoords()).first;
}

void DrawCrateText(const wchar_t* szText, HDC hdc, int posX, int posY) {
    if (hdc) {
        RECT rect = { 0, 0, posX, posY };
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 0, 0));
        DrawTextW(hdc, szText, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

void ShowCrateInfo(HWND hwnd) {
	byte* p = (byte*)0x00ABDC50;
	Powerup powerup = (Powerup)(p[0x11E]);
    Utils::LogFormat("0x00ABDC50[0x11E] Type: %d Name: %s", powerup, getCrateName(powerup));

    HDC hdc = GetDC(hwnd);
    //DrawCrateText(L"Test", hdc, 800, 600);

	// 读取地图上的箱子数据
	MapClass&map = MapClass::Instance;
    for (int i = 0; i < 0x100; i++) {
        if (map.Crates[i].CrateTimer.TimeLeft <= 0) {
            continue;
        }

        CellClass*cell = map.TryGetCellAt(map.Crates[i].Location);
        Point2D pos = GetScreenLocation(*cell);
        Utils::LogFormat("MapClass::Crates[%d] Location: (%d:%d) ScreenLocation: (%d:%d) CrateTimer.TimeLeft: %d", i, map.Crates[i].Location.X, map.Crates[i].Location.Y, pos.X, pos.Y, map.Crates[i].CrateTimer.TimeLeft);

        if (cell) {
            Powerup crate_type = Powerup::Money; 
#if 0
            if (cell->OverlayTypeIndex != -1 && OverlayTypeClass::Array[cell->OverlayTypeIndex]->Crate) {
                crate_type = (Powerup)(cell->OverlayData);
            }
#else
            crate_type = (Powerup)(cell->OverlayData);
#endif
			const wchar_t* szCrateName = getCrateName(crate_type);
            Utils::LogFormat("MapClass::Crates[%d] Type: %d", i, crate_type);
            DrawCrateText( szCrateName, hdc, pos.X, pos.Y);
        } else {
            break;
        }
    }

    ReleaseDC(hwnd, hdc);
}
