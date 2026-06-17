#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <algorithm>
#include <cwctype>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <ScenarioClass.h>
#include <Unsorted.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"

namespace {

constexpr UINT_PTR TIMER_ID_REFRESH = 1;
constexpr UINT_PTR TIMER_ID_TOPMOST = 2;
constexpr int TOOLBAR_HEIGHT = 24;
constexpr int BUTTON_HEIGHT = 24;
constexpr int BUTTON_GAP = 1;
constexpr int MENU_ITEM_HEIGHT = 22;
constexpr int MENU_MAX_VISIBLE = 10;
constexpr int ENEMY_PANEL_WIDTH = 190;
constexpr int ENEMY_PANEL_HEIGHT = 300;

HWND g_hwndCustomToolbar = NULL;
HWND g_hwndEnemyInfo = NULL;
DWORD g_toolbarThreadId = 0;
bool g_exiting = false;
std::vector<std::wstring> g_quickMessages;

enum class DropdownKind {
	None,
	QuickMessage,
	CrateArea,
};

DropdownKind g_openDropdown = DropdownKind::None;

enum ToolbarButtonId {
	ButtonToggleTurn = 1,
	ButtonToggleInfo = 2,
	ButtonScanCannon = 3,
	ButtonPickCrate = 4,
	ButtonRouteCrate = 5,
	ButtonFormationSquare = 6,
	ButtonFormationVertical = 7,
	ButtonFormationHorizontal = 8,
};

struct ToolbarButton {
	int Id;
	RECT Rect;
	const wchar_t* Text;
	bool Active;
};

struct AreaPreset {
	short Left;
	short Top;
	short Right;
	short Bottom;
	std::wstring Label;
};

struct ToolbarLayout {
	int Width;
	int Height;
	int ToolbarTop;
	RECT QuickMessageRect;
	RECT AreaPresetRect;
	std::vector<ToolbarButton> Buttons;
	std::vector<RECT> QuickMessageItems;
	std::vector<RECT> AreaPresetItems;
};

std::vector<AreaPreset> g_areaPresets;

bool ContainsPoint(const RECT& rect, int x, int y) {
	return x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom;
}

std::wstring DecodeMultiByte(const char* text, int length, UINT codePage, DWORD flags) {
	if (!text || length <= 0) {
		return std::wstring();
	}

	const int needed = MultiByteToWideChar(codePage, flags, text, length, NULL, 0);
	if (needed <= 0) {
		return std::wstring();
	}

	std::wstring result(static_cast<size_t>(needed), L'\0');
	MultiByteToWideChar(codePage, flags, text, length, &result[0], needed);
	return result;
}

std::wstring DecodeConfigBytes(const std::vector<char>& bytes) {
	if (bytes.empty()) {
		return std::wstring();
	}

	const unsigned char* data = reinterpret_cast<const unsigned char*>(bytes.data());
	if (bytes.size() >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) {
		return DecodeMultiByte(bytes.data() + 3, static_cast<int>(bytes.size() - 3), CP_UTF8, 0);
	}

	if (bytes.size() >= 2 && data[0] == 0xFF && data[1] == 0xFE) {
		const size_t charCount = (bytes.size() - 2) / sizeof(wchar_t);
		return std::wstring(reinterpret_cast<const wchar_t*>(bytes.data() + 2), charCount);
	}

	if (bytes.size() >= 2 && data[0] == 0xFE && data[1] == 0xFF) {
		std::wstring result;
		for (size_t i = 2; i + 1 < bytes.size(); i += 2) {
			result.push_back(static_cast<wchar_t>((data[i] << 8) | data[i + 1]));
		}
		return result;
	}

	std::wstring utf8 = DecodeMultiByte(bytes.data(), static_cast<int>(bytes.size()), CP_UTF8, MB_ERR_INVALID_CHARS);
	if (!utf8.empty()) {
		return utf8;
	}

	return DecodeMultiByte(bytes.data(), static_cast<int>(bytes.size()), CP_ACP, 0);
}

std::wstring Trim(const std::wstring& value) {
	size_t begin = 0;
	while (begin < value.size() && iswspace(value[begin])) {
		++begin;
	}

	size_t end = value.size();
	while (end > begin && iswspace(value[end - 1])) {
		--end;
	}

	return value.substr(begin, end - begin);
}

bool ReadConfigText(std::wstring* outText) {
	const char* path = Config::getConfigFilePath();
	HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		return false;
	}

	DWORD highSize = 0;
	const DWORD lowSize = GetFileSize(file, &highSize);
	if (lowSize == INVALID_FILE_SIZE || highSize != 0 || lowSize == 0) {
		CloseHandle(file);
		return false;
	}

	std::vector<char> bytes(lowSize);
	DWORD readSize = 0;
	const BOOL ok = ReadFile(file, bytes.data(), lowSize, &readSize, NULL);
	CloseHandle(file);
	if (!ok || readSize == 0) {
		return false;
	}

	bytes.resize(readSize);
	*outText = DecodeConfigBytes(bytes);
	return !outText->empty();
}

std::wstring CurrentMapId() {
	__try {
		if (ScenarioClass::Instance && ScenarioClass::Instance->FileName[0]) {
			return DecodeMultiByte(
				ScenarioClass::Instance->FileName,
				static_cast<int>(strlen(ScenarioClass::Instance->FileName)),
				CP_ACP,
				0);
		}

		if (Game::ScenarioName && Game::ScenarioName[0]) {
			return DecodeMultiByte(
				Game::ScenarioName,
				static_cast<int>(strlen(Game::ScenarioName)),
				CP_ACP,
				0);
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}

	return L"mapid";
}

bool IsConfigSection(const std::wstring& line, const wchar_t* name) {
	if (line.size() < 3 || line.front() != L'[' || line.back() != L']') {
		return false;
	}

	const std::wstring section = Trim(line.substr(1, line.size() - 2));
	return _wcsicmp(section.c_str(), name) == 0;
}

std::vector<std::wstring> SplitByPipe(const std::wstring& text) {
	std::vector<std::wstring> items;
	size_t begin = 0;
	while (begin <= text.size()) {
		const size_t separator = text.find(L'|', begin);
		std::wstring item = separator == std::wstring::npos
			? text.substr(begin)
			: text.substr(begin, separator - begin);
		item = Trim(item);
		if (!item.empty()) {
			items.push_back(item);
		}

		if (separator == std::wstring::npos) {
			break;
		}
		begin = separator + 1;
	}

	return items;
}

void LoadQuickMessages() {
	g_quickMessages.clear();

	std::wstring text;
	if (!ReadConfigText(&text)) {
		Utils::Log("CustomToolbar quick messages config read failed.");
		return;
	}

	bool inSection = false;
	size_t offset = 0;
	while (offset <= text.size()) {
		const size_t next = text.find_first_of(L"\r\n", offset);
		std::wstring line = next == std::wstring::npos
			? text.substr(offset)
			: text.substr(offset, next - offset);
		if (next == std::wstring::npos) {
			offset = text.size() + 1;
		} else {
			offset = next + 1;
			if (offset < text.size() && text[next] == L'\r' && text[offset] == L'\n') {
				++offset;
			}
		}

		line = Trim(line);
		if (line.empty() || line[0] == L';' || line[0] == L'#') {
			continue;
		}
		if (line.front() == L'[' && line.back() == L']') {
			inSection = IsConfigSection(line, L"chat");
			continue;
		}
		if (!inSection) {
			continue;
		}

		const size_t equals = line.find(L'=');
		if (equals == std::wstring::npos) {
			continue;
		}

		const std::wstring key = Trim(line.substr(0, equals));
		const std::wstring value = Trim(line.substr(equals + 1));
		if (_wcsicmp(key.c_str(), L"msg") == 0) {
			g_quickMessages = SplitByPipe(value);
			break;
		}
	}

	Utils::LogFormat("CustomToolbar quick messages loaded: %d", static_cast<int>(g_quickMessages.size()));
}

bool ParseAreaNumbers(const std::wstring& text, AreaPreset* preset) {
	if (!preset) {
		return false;
	}

	const wchar_t* cursor = text.c_str();
	long values[4] = {};
	for (int i = 0; i < 4; ++i) {
		while (*cursor && (*cursor == L',' || iswspace(*cursor))) {
			++cursor;
		}
		if (!*cursor) {
			return false;
		}

		wchar_t* end = nullptr;
		values[i] = wcstol(cursor, &end, 10);
		if (end == cursor) {
			return false;
		}
		cursor = end;
	}

	const short left = static_cast<short>(std::min(values[0], values[2]));
	const short top = static_cast<short>(std::min(values[1], values[3]));
	const short right = static_cast<short>(std::max(values[0], values[2]));
	const short bottom = static_cast<short>(std::max(values[1], values[3]));
	preset->Left = left;
	preset->Top = top;
	preset->Right = right;
	preset->Bottom = bottom;

	wchar_t label[96] = {};
	swprintf_s(label, _countof(label), L"%d,%d - %d,%d", left, top, right, bottom);
	preset->Label = label;
	return true;
}

bool IsAreaMapKeyMatch(const std::wstring& key, const std::wstring& mapId) {
	return _wcsicmp(key.c_str(), mapId.c_str()) == 0
		|| _wcsicmp(key.c_str(), L"mapid") == 0
		|| _wcsicmp(key.c_str(), L"default") == 0;
}

void LoadAreaPresets() {
	g_areaPresets.clear();

	std::wstring text;
	if (!ReadConfigText(&text)) {
		Utils::Log("CustomToolbar area presets config read failed.");
		return;
	}

	const std::wstring mapId = CurrentMapId();
	bool inSection = false;
	size_t offset = 0;
	while (offset <= text.size()) {
		const size_t next = text.find_first_of(L"\r\n", offset);
		std::wstring line = next == std::wstring::npos
			? text.substr(offset)
			: text.substr(offset, next - offset);
		if (next == std::wstring::npos) {
			offset = text.size() + 1;
		} else {
			offset = next + 1;
			if (offset < text.size() && text[next] == L'\r' && text[offset] == L'\n') {
				++offset;
			}
		}

		line = Trim(line);
		if (line.empty() || line[0] == L';' || line[0] == L'#') {
			continue;
		}
		if (line.front() == L'[' && line.back() == L']') {
			inSection = IsConfigSection(line, L"cratearea");
			continue;
		}
		if (!inSection) {
			continue;
		}

		const size_t equals = line.find(L'=');
		if (equals == std::wstring::npos) {
			continue;
		}

		const std::wstring key = Trim(line.substr(0, equals));
		const std::wstring value = Trim(line.substr(equals + 1));
		if (!IsAreaMapKeyMatch(key, mapId)) {
			continue;
		}

		const auto groups = SplitByPipe(value);
		for (const auto& group : groups) {
			AreaPreset preset = {};
			if (ParseAreaNumbers(group, &preset)) {
				g_areaPresets.push_back(preset);
			}
		}
	}

	Utils::LogFormat("CustomToolbar area presets loaded: %d", static_cast<int>(g_areaPresets.size()));
}

int VisibleQuickMessageCount() {
	return std::min(static_cast<int>(g_quickMessages.size()), MENU_MAX_VISIBLE);
}

int VisibleAreaPresetCount() {
	return std::min(static_cast<int>(g_areaPresets.size()), MENU_MAX_VISIBLE);
}

int CurrentMenuHeight() {
	int count = 0;
	if (g_openDropdown == DropdownKind::QuickMessage) {
		count = VisibleQuickMessageCount();
	} else if (g_openDropdown == DropdownKind::CrateArea) {
		count = VisibleAreaPresetCount();
	}

	return count > 0 ? count * MENU_ITEM_HEIGHT + 4 : 0;
}

ToolbarLayout BuildLayout(int width) {
	ToolbarLayout layout = {};
	layout.Width = std::max(320, width);
	layout.ToolbarTop = CurrentMenuHeight();
	layout.Height = layout.ToolbarTop + TOOLBAR_HEIGHT;

	int x = 0;
	auto addButton = [&layout, &x](int id, int width, const wchar_t* text, bool active) {
		layout.Buttons.push_back({ id, { x, layout.ToolbarTop, x + width, layout.ToolbarTop + BUTTON_HEIGHT }, text, active });
		x += width + BUTTON_GAP;
	};

	addButton(ButtonToggleTurn, 62,
		Config::isGrandCannonAutoTurnEnabled() ? L"自转:开" : L"自转:关",
		Config::isGrandCannonAutoTurnEnabled());
	addButton(ButtonToggleInfo, 62,
		Config::isShowEnemyInfo() ? L"敌情:开" : L"敌情:关",
		Config::isShowEnemyInfo());
	addButton(ButtonScanCannon, 56, L"扫巨炮", true);
	addButton(ButtonPickCrate, 56, L"捡箱", true);

	const wchar_t* routeText = L"跑图捡箱";
	if (IsRouteCrateCaptureActive()) {
		routeText = L"完成区域";
	} else if (IsRouteCrateRunActive()) {
		routeText = L"停止跑图";
	}
	addButton(ButtonRouteCrate, 70, routeText, IsRouteCrateCaptureActive() || IsRouteCrateRunActive());

	layout.AreaPresetRect = { x, layout.ToolbarTop, x + 112, layout.ToolbarTop + BUTTON_HEIGHT };
	x += 112 + BUTTON_GAP;

	addButton(ButtonFormationSquare, 52, L"阵口", true);
	addButton(ButtonFormationVertical, 52, L"阵一", true);
	addButton(ButtonFormationHorizontal, 52, L"阵丨", true);

	layout.QuickMessageRect = { x, layout.ToolbarTop, layout.Width, layout.ToolbarTop + BUTTON_HEIGHT };
	if (layout.QuickMessageRect.right - layout.QuickMessageRect.left < 96) {
		layout.QuickMessageRect.left = std::max(0, layout.Width - 96);
	}

	if (g_openDropdown == DropdownKind::QuickMessage) {
		const int count = VisibleQuickMessageCount();
		for (int i = 0; i < count; ++i) {
			layout.QuickMessageItems.push_back({
				layout.QuickMessageRect.left,
				2 + i * MENU_ITEM_HEIGHT,
				layout.QuickMessageRect.right,
				2 + (i + 1) * MENU_ITEM_HEIGHT
				});
		}
	} else if (g_openDropdown == DropdownKind::CrateArea) {
		const int count = VisibleAreaPresetCount();
		for (int i = 0; i < count; ++i) {
			layout.AreaPresetItems.push_back({
				layout.AreaPresetRect.left,
				2 + i * MENU_ITEM_HEIGHT,
				layout.AreaPresetRect.right,
				2 + (i + 1) * MENU_ITEM_HEIGHT
				});
		}
	}

	return layout;
}

HFONT CreateToolbarFont() {
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
		L"Microsoft YaHei");
}

void FillSolid(HDC hdc, const RECT& rect, COLORREF color) {
	HBRUSH brush = CreateSolidBrush(color);
	FillRect(hdc, &rect, brush);
	DeleteObject(brush);
}

void DrawCenteredText(HDC hdc, const RECT& rect, const wchar_t* text, COLORREF color) {
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, color);
	RECT textRect = rect;
	DrawTextW(hdc, text ? text : L"", -1, &textRect,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void DrawButton(HDC hdc, const ToolbarButton& button) {
	FillSolid(hdc, button.Rect, button.Active ? RGB(54, 78, 58) : RGB(48, 50, 52));
	DrawCenteredText(hdc, button.Rect, button.Text, RGB(240, 240, 240));
}

void DrawDropdownButton(HDC hdc, const RECT& rect, const wchar_t* text, bool active) {
	FillSolid(hdc, rect, active ? RGB(62, 72, 88) : RGB(46, 50, 56));
	DrawCenteredText(hdc, rect, text, RGB(240, 240, 240));
}

void DrawMenuItems(HDC hdc, const std::vector<RECT>& rects, const std::vector<std::wstring>& texts) {
	for (size_t i = 0; i < rects.size(); ++i) {
		FillSolid(hdc, rects[i], (i % 2) == 0 ? RGB(31, 34, 37) : RGB(38, 41, 44));
		if (i < texts.size()) {
			DrawCenteredText(hdc, rects[i], texts[i].c_str(), RGB(242, 242, 242));
		}
	}
}

void DrawToolbar(HWND hwnd) {
	RECT client = {};
	GetClientRect(hwnd, &client);
	const ToolbarLayout layout = BuildLayout(client.right - client.left);

	PAINTSTRUCT ps = {};
	HDC hdc = BeginPaint(hwnd, &ps);
	HFONT font = CreateToolbarFont();
	HGDIOBJ oldFont = SelectObject(hdc, font);

	FillSolid(hdc, client, RGB(22, 24, 26));

	if (g_openDropdown == DropdownKind::QuickMessage && !layout.QuickMessageItems.empty()) {
		DrawMenuItems(hdc, layout.QuickMessageItems, g_quickMessages);
	} else if (g_openDropdown == DropdownKind::CrateArea && !layout.AreaPresetItems.empty()) {
		std::vector<std::wstring> texts;
		for (const auto& preset : g_areaPresets) {
			texts.push_back(preset.Label);
		}
		DrawMenuItems(hdc, layout.AreaPresetItems, texts);
	}

	for (const auto& button : layout.Buttons) {
		DrawButton(hdc, button);
	}

	DrawDropdownButton(hdc, layout.AreaPresetRect,
		g_areaPresets.empty() ? L"无预设区域" : L"预设区域",
		g_openDropdown == DropdownKind::CrateArea);
	DrawDropdownButton(hdc, layout.QuickMessageRect,
		g_quickMessages.empty() ? L"无快捷消息" : L"快捷消息",
		g_openDropdown == DropdownKind::QuickMessage);

	SelectObject(hdc, oldFont);
	DeleteObject(font);
	EndPaint(hwnd, &ps);
}

void Topmost(HWND hwnd) {
	if (hwnd) {
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
}

int ToolbarWidthForGame(HWND gameHwnd) {
	RECT client = {};
	if (!gameHwnd || !GetClientRect(gameHwnd, &client)) {
		return 760;
	}

	const int width = client.right - client.left;
	return std::min(760, std::max(320, width - 4));
}

void RepositionToolbar() {
	if (!g_hwndCustomToolbar) {
		return;
	}

	HWND gameHwnd = GetMainWindowForProcessId(GetCurrentProcessId());
	RECT client = {};
	POINT topLeft = { 0, 0 };
	if (!gameHwnd || !GetClientRect(gameHwnd, &client) || !ClientToScreen(gameHwnd, &topLeft)) {
		return;
	}

	const int clientWidth = client.right - client.left;
	const int clientHeight = client.bottom - client.top;
	const int toolbarWidth = ToolbarWidthForGame(gameHwnd);
	const ToolbarLayout layout = BuildLayout(toolbarWidth);
	const int x = topLeft.x + std::max(0, (clientWidth - toolbarWidth) / 2);
	const int y = topLeft.y + std::max(0, clientHeight - layout.Height);

	SetWindowPos(g_hwndCustomToolbar, HWND_TOPMOST, x, y, toolbarWidth, layout.Height,
		SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void RepositionEnemyInfo() {
	if (!g_hwndEnemyInfo || !Config::isShowEnemyInfo()) {
		return;
	}

	HWND gameHwnd = GetMainWindowForProcessId(GetCurrentProcessId());
	RECT client = {};
	POINT topLeft = { 0, 0 };
	if (!gameHwnd || !GetClientRect(gameHwnd, &client) || !ClientToScreen(gameHwnd, &topLeft)) {
		return;
	}

	const int clientWidth = client.right - client.left;
	const int clientHeight = client.bottom - client.top;
	const int bottomReserved = Config::isCustomToolbarEnabled() ? TOOLBAR_HEIGHT + 4 : 4;
	const int x = topLeft.x + std::max(0, clientWidth - ENEMY_PANEL_WIDTH - 6);
	const int y = topLeft.y + std::max(0, clientHeight - ENEMY_PANEL_HEIGHT - bottomReserved);
	SetWindowPos(g_hwndEnemyInfo, HWND_TOPMOST, x, y, ENEMY_PANEL_WIDTH, ENEMY_PANEL_HEIGHT,
		SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void RefreshEnemyInfoWindow() {
	if (!g_hwndEnemyInfo) {
		return;
	}

	if (Config::isShowEnemyInfo()) {
		ShowWindow(g_hwndEnemyInfo, SW_SHOWNOACTIVATE);
		RepositionEnemyInfo();
		InvalidateRect(g_hwndEnemyInfo, NULL, TRUE);
	} else {
		ShowWindow(g_hwndEnemyInfo, SW_HIDE);
	}
}

void HandleButtonClick(int id, HWND hwnd) {
	switch (id) {
	case ButtonToggleTurn: {
		const bool enabled = !Config::isGrandCannonAutoTurnEnabled();
		Config::setGrandCannonAutoTurnEnabled(enabled);
		Utils::LogFormat("CustomToolbar click: AutoTurn=%d", enabled ? 1 : 0);
		break;
	}
	case ButtonToggleInfo: {
		const bool enabled = !Config::isShowEnemyInfo();
		Config::setShowEnemyInfo(enabled);
		Utils::LogFormat("CustomToolbar click: ShowEnemyInfo=%d", enabled ? 1 : 0);
		RefreshEnemyInfoWindow();
		break;
	}
	case ButtonScanCannon:
		Utils::Log("CustomToolbar click: ScanGrandCannon");
		ForceGrandCannonScan();
		break;
	case ButtonPickCrate:
		Utils::Log("CustomToolbar click: PickCrate");
		StartOptimalCratePickup();
		break;
	case ButtonRouteCrate:
		Utils::Log("CustomToolbar click: RouteCrate");
		ToggleRouteCratePickup();
		break;
	case ButtonFormationSquare:
		Utils::Log("CustomToolbar click: FormationSquare");
		ApplySelectedFormationSquare();
		break;
	case ButtonFormationVertical:
		Utils::Log("CustomToolbar click: FormationVertical");
		ApplySelectedFormationVertical();
		break;
	case ButtonFormationHorizontal:
		Utils::Log("CustomToolbar click: FormationHorizontal");
		ApplySelectedFormationHorizontal();
		break;
	default:
		break;
	}

	g_openDropdown = DropdownKind::None;
	RepositionToolbar();
	InvalidateRect(hwnd, NULL, TRUE);
	if (g_hwndTipWindow) {
		InvalidateRect(g_hwndTipWindow, NULL, TRUE);
	}
}

void ToggleQuickMessages(HWND hwnd) {
	if (g_quickMessages.empty()) {
		LoadQuickMessages();
	}
	g_openDropdown = g_openDropdown == DropdownKind::QuickMessage
		? DropdownKind::None
		: (g_quickMessages.empty() ? DropdownKind::None : DropdownKind::QuickMessage);
	RepositionToolbar();
	InvalidateRect(hwnd, NULL, TRUE);
}

void ToggleAreaPresets(HWND hwnd) {
	if (g_areaPresets.empty()) {
		LoadAreaPresets();
	}
	g_openDropdown = g_openDropdown == DropdownKind::CrateArea
		? DropdownKind::None
		: (g_areaPresets.empty() ? DropdownKind::None : DropdownKind::CrateArea);
	RepositionToolbar();
	InvalidateRect(hwnd, NULL, TRUE);
}

void HandleToolbarClick(HWND hwnd, int x, int y) {
	RECT client = {};
	GetClientRect(hwnd, &client);
	const ToolbarLayout layout = BuildLayout(client.right - client.left);

	if (g_openDropdown == DropdownKind::QuickMessage) {
		for (size_t i = 0; i < layout.QuickMessageItems.size(); ++i) {
			if (ContainsPoint(layout.QuickMessageItems[i], x, y) && i < g_quickMessages.size()) {
				const std::wstring message = g_quickMessages[i];
				g_openDropdown = DropdownKind::None;
				RepositionToolbar();
				InvalidateRect(hwnd, NULL, TRUE);
				Chat(message.c_str());
				return;
			}
		}
	} else if (g_openDropdown == DropdownKind::CrateArea) {
		for (size_t i = 0; i < layout.AreaPresetItems.size(); ++i) {
			if (ContainsPoint(layout.AreaPresetItems[i], x, y) && i < g_areaPresets.size()) {
				const AreaPreset preset = g_areaPresets[i];
				g_openDropdown = DropdownKind::None;
				RepositionToolbar();
				InvalidateRect(hwnd, NULL, TRUE);
				StartRouteCratePickupInArea(preset.Left, preset.Top, preset.Right, preset.Bottom);
				return;
			}
		}
	}

	if (ContainsPoint(layout.QuickMessageRect, x, y)) {
		ToggleQuickMessages(hwnd);
		return;
	}

	if (ContainsPoint(layout.AreaPresetRect, x, y)) {
		ToggleAreaPresets(hwnd);
		return;
	}

	g_openDropdown = DropdownKind::None;
	for (const auto& button : layout.Buttons) {
		if (ContainsPoint(button.Rect, x, y)) {
			HandleButtonClick(button.Id, hwnd);
			return;
		}
	}

	RepositionToolbar();
	InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK CustomToolbarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_TIMER:
		if (wParam == TIMER_ID_REFRESH) {
			RepositionToolbar();
			RefreshEnemyInfoWindow();
			InvalidateRect(hwnd, NULL, TRUE);
		} else if (wParam == TIMER_ID_TOPMOST) {
			Topmost(hwnd);
			Topmost(g_hwndEnemyInfo);
		}
		break;
	case WM_PAINT:
		DrawToolbar(hwnd);
		break;
	case WM_LBUTTONUP:
		HandleToolbarClick(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	case WM_ERASEBKGND:
		return 1;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		if (hwnd == g_hwndCustomToolbar) {
			KillTimer(hwnd, TIMER_ID_REFRESH);
			KillTimer(hwnd, TIMER_ID_TOPMOST);
			g_hwndCustomToolbar = NULL;
		}
		if (g_exiting && !g_hwndEnemyInfo) {
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

LRESULT CALLBACK EnemyInfoWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_TIMER:
		if (wParam == TIMER_ID_REFRESH) {
			RepositionEnemyInfo();
			InvalidateRect(hwnd, NULL, TRUE);
		} else if (wParam == TIMER_ID_TOPMOST) {
			Topmost(hwnd);
		}
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps = {};
		HDC hdc = BeginPaint(hwnd, &ps);
		ShowEnemyPlayerInfo(hdc);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	case WM_ERASEBKGND:
		return 1;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		if (hwnd == g_hwndEnemyInfo) {
			KillTimer(hwnd, TIMER_ID_REFRESH);
			KillTimer(hwnd, TIMER_ID_TOPMOST);
			g_hwndEnemyInfo = NULL;
		}
		if (g_exiting && !g_hwndCustomToolbar) {
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

void RegisterWindowClasses() {
	WNDCLASSW toolbarClass = {};
	toolbarClass.lpfnWndProc = CustomToolbarWndProc;
	toolbarClass.hInstance = g_thisModule;
	toolbarClass.lpszClassName = L"RA2CustomToolbar";
	RegisterClassW(&toolbarClass);

	WNDCLASSW enemyClass = {};
	enemyClass.lpfnWndProc = EnemyInfoWndProc;
	enemyClass.hInstance = g_thisModule;
	enemyClass.lpszClassName = L"RA2EnemyInfoPanel";
	RegisterClassW(&enemyClass);
}

void CreateToolbarWindow() {
	if (!Config::isCustomToolbarEnabled() || g_hwndCustomToolbar) {
		return;
	}

	HWND hwnd = CreateWindowExW(
		WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
		L"RA2CustomToolbar",
		L"RA2CustomToolbar",
		WS_POPUP | WS_VISIBLE,
		0,
		0,
		760,
		TOOLBAR_HEIGHT,
		NULL,
		NULL,
		g_thisModule,
		NULL);

	if (!hwnd) {
		Utils::Log(GetLastError(), "CustomToolbar window creation failed.");
		return;
	}

	g_hwndCustomToolbar = hwnd;
	SetLayeredWindowAttributes(hwnd, 0, 238, LWA_ALPHA);
	SetTimer(hwnd, TIMER_ID_REFRESH, 300, NULL);
	SetTimer(hwnd, TIMER_ID_TOPMOST, 2000, NULL);
	RepositionToolbar();
}

void CreateEnemyInfoWindow() {
	if (g_hwndEnemyInfo) {
		return;
	}

	HWND hwnd = CreateWindowExW(
		WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
		L"RA2EnemyInfoPanel",
		L"RA2EnemyInfoPanel",
		WS_POPUP | (Config::isShowEnemyInfo() ? WS_VISIBLE : 0),
		0,
		0,
		ENEMY_PANEL_WIDTH,
		ENEMY_PANEL_HEIGHT,
		NULL,
		NULL,
		g_thisModule,
		NULL);

	if (!hwnd) {
		Utils::Log(GetLastError(), "EnemyInfoPanel window creation failed.");
		return;
	}

	g_hwndEnemyInfo = hwnd;
	SetLayeredWindowAttributes(hwnd, 0, 218, LWA_ALPHA);
	SetTimer(hwnd, TIMER_ID_REFRESH, 500, NULL);
	SetTimer(hwnd, TIMER_ID_TOPMOST, 2000, NULL);
	RefreshEnemyInfoWindow();
}

unsigned __stdcall ThreadProcCreateCustomToolbar(void*) {
	g_toolbarThreadId = GetCurrentThreadId();
	g_exiting = false;
	RegisterWindowClasses();
	LoadQuickMessages();
	LoadAreaPresets();
	CreateToolbarWindow();
	CreateEnemyInfoWindow();

	if (!g_hwndCustomToolbar && !g_hwndEnemyInfo) {
		Utils::Log("CustomToolbar no window created.");
		return 0;
	}

	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	g_hwndCustomToolbar = NULL;
	g_hwndEnemyInfo = NULL;
	g_toolbarThreadId = 0;
	g_exiting = false;
	return 0;
}

}

void InitCustomToolbar() {
	if (g_toolbarThreadId != 0 || (!Config::isCustomToolbarEnabled() && !Config::isShowEnemyInfo())) {
		return;
	}

	Utils::Log("InitCustomToolbar!");
	_beginthreadex(NULL, 0, ThreadProcCreateCustomToolbar, NULL, 0, NULL);
}

void UnInitCustomToolbar() {
	if (!g_hwndCustomToolbar && !g_hwndEnemyInfo && g_toolbarThreadId == 0) {
		return;
	}

	g_exiting = true;
	if (g_hwndCustomToolbar) {
		PostMessage(g_hwndCustomToolbar, WM_CLOSE, 0, 0);
	}
	if (g_hwndEnemyInfo) {
		PostMessage(g_hwndEnemyInfo, WM_CLOSE, 0, 0);
	}
	if (!g_hwndCustomToolbar && !g_hwndEnemyInfo && g_toolbarThreadId != 0) {
		PostThreadMessage(g_toolbarThreadId, WM_QUIT, 0, 0);
	}

	Utils::Log("UnInitCustomToolbar!");
}
