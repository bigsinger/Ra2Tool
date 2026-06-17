#include <windows.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <HouseClass.h>
#include <UnitClass.h>
#include <Surface.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"
#include "OverlayToolbar/OverlayMixer.h"

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
		wcscpy_s(buffer, bufferCount, L"未知");
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

	wcscpy_s(buffer, bufferCount, L"未知");
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

void BlendPixel(std::vector<unsigned char>* pixels, int width, int height, int x, int y,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	if (!pixels || a == 0 || x < 0 || y < 0 || x >= width || y >= height) {
		return;
	}

	unsigned char* dst = pixels->data() + (y * width + x) * 4;
	if (a == 255 || dst[3] == 0) {
		dst[0] = r;
		dst[1] = g;
		dst[2] = b;
		dst[3] = a;
		return;
	}

	const unsigned int inv = 255 - a;
	dst[0] = static_cast<unsigned char>((r * a + dst[0] * inv) / 255);
	dst[1] = static_cast<unsigned char>((g * a + dst[1] * inv) / 255);
	dst[2] = static_cast<unsigned char>((b * a + dst[2] * inv) / 255);
	dst[3] = std::max(dst[3], a);
}

void FillRectRGBA(std::vector<unsigned char>* pixels, int width, int height, const RECT& rect,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	const int left = static_cast<int>(std::max<LONG>(0, rect.left));
	const int top = static_cast<int>(std::max<LONG>(0, rect.top));
	const int right = static_cast<int>(std::min<LONG>(width, rect.right));
	const int bottom = static_cast<int>(std::min<LONG>(height, rect.bottom));

	for (int y = top; y < bottom; ++y) {
		for (int x = left; x < right; ++x) {
			BlendPixel(pixels, width, height, x, y, r, g, b, a);
		}
	}
}

HFONT CreateEnemyInfoFont() {
	return CreateFontW(
		-14,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"Microsoft YaHei UI");
}

void DrawTextRGBA(std::vector<unsigned char>* pixels, int width, int height, const RECT& rect,
	const wchar_t* text, COLORREF color) {
	if (!pixels || !text || !*text || rect.right <= rect.left || rect.bottom <= rect.top) {
		return;
	}

	const int textWidth = rect.right - rect.left;
	const int textHeight = rect.bottom - rect.top;
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = textWidth;
	bmi.bmiHeader.biHeight = -textHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* dibBits = nullptr;
	HDC screen = GetDC(NULL);
	HDC hdc = CreateCompatibleDC(screen);
	HBITMAP bitmap = CreateDIBSection(screen, &bmi, DIB_RGB_COLORS, &dibBits, NULL, 0);
	ReleaseDC(NULL, screen);
	if (!hdc || !bitmap || !dibBits) {
		if (bitmap) {
			DeleteObject(bitmap);
		}
		if (hdc) {
			DeleteDC(hdc);
		}
		return;
	}

	memset(dibBits, 0, static_cast<size_t>(textWidth * textHeight * 4));
	HGDIOBJ oldBitmap = SelectObject(hdc, bitmap);
	HFONT font = CreateEnemyInfoFont();
	HGDIOBJ oldFont = SelectObject(hdc, font);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, color);
	RECT localRect { 0, 0, textWidth, textHeight };
	DrawTextW(hdc, text, -1, &localRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	auto* bgra = reinterpret_cast<unsigned char*>(dibBits);
	for (int y = 0; y < textHeight; ++y) {
		for (int x = 0; x < textWidth; ++x) {
			const unsigned char* src = bgra + (y * textWidth + x) * 4;
			const unsigned char b = src[0];
			const unsigned char g = src[1];
			const unsigned char r = src[2];
			if (r || g || b) {
				BlendPixel(pixels, width, height, rect.left + x, rect.top + y, r, g, b, 255);
			}
		}
	}

	SelectObject(hdc, oldFont);
	SelectObject(hdc, oldBitmap);
	DeleteObject(font);
	DeleteObject(bitmap);
	DeleteDC(hdc);
}

int CountEnemyHouses() {
	int enemyCount = 0;
	for (int i = 0; i < HouseClass::Array.Count; ++i) {
		if (IsEnemyHouse(HouseClass::Array.GetItem(i))) {
			++enemyCount;
		}
	}
	return enemyCount;
}

void DrawEnemyInfoOverlayUnsafe() {
	if (!Config::isShowEnemyInfo()) {
		return;
	}

	DSurface* surface = DSurface::Composite;
	if (!surface) {
		return;
	}

	const int surfaceWidth = surface->GetWidth();
	const int surfaceHeight = surface->GetHeight();
	if (surfaceWidth <= 0 || surfaceHeight <= 0) {
		return;
	}

	std::vector<EnemyStats> enemies;
	for (int i = 0; i < HouseClass::Array.Count; ++i) {
		auto house = HouseClass::Array.GetItem(i);
		if (IsEnemyHouse(house)) {
			enemies.push_back(BuildStats(house));
		}
	}

	if (enemies.empty()) {
		return;
	}

	const int panelWidth = 176;
	const int headerHeight = 22;
	const int itemHeight = 80;
	const int panelHeight = headerHeight + static_cast<int>(enemies.size()) * itemHeight + 6;
	std::vector<unsigned char> pixels(static_cast<size_t>(panelWidth * panelHeight * 4), 0);

	RECT panel { 0, 0, panelWidth, panelHeight };
	FillRectRGBA(&pixels, panelWidth, panelHeight, panel, 14, 16, 16, 166);

	RECT headerRect { 8, 3, panelWidth - 8, headerHeight };
	DrawTextRGBA(&pixels, panelWidth, panelHeight, headerRect, L"敌情信息", RGB(238, 238, 238));

	for (size_t i = 0; i < enemies.size(); ++i) {
		auto& stats = enemies[i];
		auto house = stats.House;
		const int y = headerHeight + static_cast<int>(i) * itemHeight;
		RECT colorRect { 8, y + 7, 22, y + 21 };
		FillRectRGBA(&pixels, panelWidth, panelHeight, colorRect,
			house->Color.R, house->Color.G, house->Color.B, 230);

		wchar_t name[32] = {};
		CopyHouseName(house, name, _countof(name));

		wchar_t title[64] = {};
		swprintf_s(title, _countof(title), L"玩家: %ls", name);
		RECT titleRect { 28, y + 3, panelWidth - 8, y + 24 };
		DrawTextRGBA(&pixels, panelWidth, panelHeight, titleRect, title, RGB(246, 246, 246));

		wchar_t moneyLine[64] = {};
		swprintf_s(moneyLine, _countof(moneyLine), L"金钱: %d", house->Balance);
		RECT moneyRect { 8, y + 23, panelWidth - 8, y + 42 };
		DrawTextRGBA(&pixels, panelWidth, panelHeight, moneyRect, moneyLine, RGB(232, 232, 232));

		wchar_t unitLine[96] = {};
		swprintf_s(unitLine, _countof(unitLine), L"坦克: %d  矿车: %d", stats.Tanks, stats.Harvesters);
		RECT unitRect { 8, y + 40, panelWidth - 8, y + 59 };
		DrawTextRGBA(&pixels, panelWidth, panelHeight, unitRect, unitLine, RGB(232, 232, 232));

		wchar_t countLine[96] = {};
		swprintf_s(countLine, _countof(countLine), L"建筑: %d  步兵: %d", house->OwnedBuildings, house->OwnedInfantry);
		RECT countRect { 8, y + 57, panelWidth - 8, y + 76 };
		DrawTextRGBA(&pixels, panelWidth, panelHeight, countRect, countLine, RGB(232, 232, 232));
	}

	const int margin = 6;
	const int bottomReserved = Config::isCustomToolbarEnabled() ? 30 : margin;
	const int x = std::max(margin, surfaceWidth - panelWidth - margin);
	const int y = std::max(margin, surfaceHeight - panelHeight - bottomReserved);
	DrawOverlayRGBA(x, y, panelWidth, panelHeight, pixels.data());
}

}

void DrawEnemyInfoOverlay() {
	__try {
		DrawEnemyInfoOverlayUnsafe();
	} __except (EXCEPTION_EXECUTE_HANDLER) {
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
		const wchar_t* header = L"颜色  玩家                金钱   坦克  矿车  建筑  步兵";
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
