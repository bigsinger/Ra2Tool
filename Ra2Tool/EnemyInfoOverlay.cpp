#include <windows.h>
#include <stdio.h>
#include <HouseClass.h>
#include <UnitClass.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"

namespace {

struct EnemyStats {
	HouseClass* House;
	int Tanks;
	int Harvesters;
};

bool IsEnemyHouse(HouseClass* house) {
	auto current = HouseClass::CurrentPlayer;
	return current
		&& house
		&& house != current
		&& !house->Defeated
		&& !house->IsGameOver
		&& !house->IsNeutral()
		&& !current->IsAlliedWith(house);
}

bool IsLiveUnit(UnitClass* unit) {
	return unit
		&& unit->Type
		&& unit->Owner
		&& unit->IsAlive
		&& unit->IsOnMap
		&& !unit->InLimbo;
}

bool IsHarvester(UnitClass* unit) {
	if (!unit || !unit->Type) {
		return false;
	}

	const char* id = unit->Type->get_ID();
	return unit->Type->Harvester
		|| unit->Type->ResourceGatherer
		|| (id && (
			_stricmp(id, "HARV") == 0
			|| _stricmp(id, "HORV") == 0
			|| _stricmp(id, "CMIN") == 0
			|| _stricmp(id, "CMON") == 0
			|| _stricmp(id, "SMIN") == 0));
}

bool IsTankLike(UnitClass* unit) {
	if (!unit || !unit->Type || IsHarvester(unit)) {
		return false;
	}

	return !unit->Type->Naval
		&& !unit->Type->ConsideredAircraft
		&& !unit->Type->NonVehicle;
}

EnemyStats BuildStats(HouseClass* house) {
	EnemyStats stats = { house, 0, 0 };

	__try {
		for (int i = 0; i < UnitClass::Array.Count; ++i) {
			auto unit = UnitClass::Array.GetItem(i);
			if (!IsLiveUnit(unit) || unit->Owner != house) {
				continue;
			}

			if (IsHarvester(unit)) {
				++stats.Harvesters;
			} else if (IsTankLike(unit)) {
				++stats.Tanks;
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		stats.Tanks = 0;
		stats.Harvesters = 0;
	}

	return stats;
}

void CopyHouseName(HouseClass* house, wchar_t* buffer, size_t bufferCount) {
	if (!buffer || bufferCount == 0) {
		return;
	}

	buffer[0] = L'\0';
	if (!house) {
		wcscpy_s(buffer, bufferCount, L"\x672A\x77E5");
		return;
	}

	if (house->PlainName[0]) {
		MultiByteToWideChar(CP_ACP, 0, house->PlainName, -1, buffer, static_cast<int>(bufferCount));
		buffer[bufferCount - 1] = L'\0';
		return;
	}

	if (house->UIName[0]) {
		wcscpy_s(buffer, bufferCount, house->UIName);
		return;
	}

	const char* id = house->get_ID();
	if (id && *id) {
		MultiByteToWideChar(CP_ACP, 0, id, -1, buffer, static_cast<int>(bufferCount));
		buffer[bufferCount - 1] = L'\0';
		return;
	}

	wcscpy_s(buffer, bufferCount, L"\x672A\x77E5");
}

COLORREF ToColorRef(const ColorStruct& color) {
	return RGB(color.R, color.G, color.B);
}

void DrawPanelBackground(HDC hdc, RECT rect) {
	HBRUSH brush = CreateSolidBrush(RGB(24, 24, 24));
	FillRect(hdc, &rect, brush);
	DeleteObject(brush);

	HBRUSH frame = CreateSolidBrush(RGB(90, 90, 90));
	FrameRect(hdc, &rect, frame);
	DeleteObject(frame);
}

}

void ShowEnemyPlayerInfo(HDC hdc) {
	if (!Config::isShowEnemyInfo()) {
		return;
	}

	__try {
		int enemyCount = 0;
		for (int i = 0; i < HouseClass::Array.Count; ++i) {
			if (IsEnemyHouse(HouseClass::Array.GetItem(i))) {
				++enemyCount;
			}
		}

		if (enemyCount <= 0) {
			return;
		}

		const int rowHeight = 18;
		const int panelWidth = 380;
		const int panelHeight = 28 + enemyCount * rowHeight;
		RECT panel = { 8, 8, 8 + panelWidth, 8 + panelHeight };

		DrawPanelBackground(hdc, panel);

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(230, 230, 230));
		const wchar_t* header = L"\x989C\x8272  \x73A9\x5BB6                \x91D1\x94B1   \x5766\x514B  \x77FF\x8F66  \x5EFA\x7B51  \x6B65\x5175";
		TextOutW(hdc, 16, 14, header, static_cast<int>(wcslen(header)));

		int row = 0;
		for (int i = 0; i < HouseClass::Array.Count; ++i) {
			auto house = HouseClass::Array.GetItem(i);
			if (!IsEnemyHouse(house)) {
				continue;
			}

			auto stats = BuildStats(house);
			const int y = 34 + row * rowHeight;

			RECT colorRect = { 18, y + 3, 34, y + 15 };
			HBRUSH colorBrush = CreateSolidBrush(ToColorRef(house->Color));
			FillRect(hdc, &colorRect, colorBrush);
			DeleteObject(colorBrush);

			HBRUSH frameBrush = CreateSolidBrush(RGB(180, 180, 180));
			FrameRect(hdc, &colorRect, frameBrush);
			DeleteObject(frameBrush);

			wchar_t name[32] = {};
			CopyHouseName(house, name, _countof(name));

			wchar_t line[256] = {};
			swprintf_s(line, _countof(line), L"%-20ls %8d %5d %6d %4d %4d",
				name,
				house->Balance,
				stats.Tanks,
				stats.Harvesters,
				house->OwnedBuildings,
				house->OwnedInfantry);

			SetTextColor(hdc, RGB(245, 245, 245));
			TextOutW(hdc, 42, y, line, static_cast<int>(wcslen(line)));
			++row;
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
}
