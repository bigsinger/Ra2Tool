#include "OverlayToolbar.h"

#include "OverlayMixer.h"

#include <algorithm>
#include <cwctype>
#include <string>
#include <vector>
#include <windows.h>
#include <Surface.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Helper.h"

namespace {

constexpr DWORD HOOK_POINT = 0x004F4573;
DWORD g_hookReturn = 0x004F4578;

constexpr int TOOLBAR_HEIGHT = 24;
constexpr int MENU_ITEM_HEIGHT = 22;
constexpr int MENU_MAX_VISIBLE = 6;
constexpr int BUTTON_HEIGHT = 24;
constexpr int BUTTON_GAP = 1;

bool g_enabled = false;
bool g_hookInstalled = false;
bool g_mouseWasDown = false;
bool g_messageOpen = false;
DWORD g_lastClickTick = 0;
std::vector<std::wstring> g_quickMessages;

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

struct ToolbarLayout {
	int X;
	int Y;
	int Width;
	int Height;
	int ToolbarTop;
	RECT MessageRect;
	std::vector<ToolbarButton> Buttons;
	std::vector<RECT> MessageItems;
};

int RectWidth(const RECT& rect) {
	return rect.right - rect.left;
}

int RectHeight(const RECT& rect) {
	return rect.bottom - rect.top;
}

bool ContainsPoint(const RECT& rect, int x, int y) {
	return x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom;
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

void DrawTextRGBA(std::vector<unsigned char>* pixels, int width, int height, const RECT& rect,
	const wchar_t* text, bool centered) {
	if (!pixels || !text || !*text || RectWidth(rect) <= 0 || RectHeight(rect) <= 0) {
		return;
	}

	const int textWidth = RectWidth(rect);
	const int textHeight = RectHeight(rect);
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
	HFONT font = CreateToolbarFont();
	HGDIOBJ oldFont = SelectObject(hdc, font);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(238, 238, 238));
	RECT localRect { 2, 0, textWidth - 2, textHeight };
	const UINT flags = (centered ? DT_CENTER : DT_LEFT) | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
	DrawTextW(hdc, text, -1, &localRect, flags);

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

std::vector<std::wstring> ParseQuickMessages(const std::wstring& text) {
	bool inSection = false;
	std::wstring rawMessages;
	size_t offset = 0;

	while (offset <= text.size()) {
		size_t next = text.find_first_of(L"\r\n", offset);
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
			std::wstring section = Trim(line.substr(1, line.size() - 2));
			inSection = _wcsicmp(section.c_str(), L"chat") == 0;
			continue;
		}

		if (!inSection) {
			continue;
		}

		size_t equals = line.find(L'=');
		if (equals == std::wstring::npos) {
			continue;
		}

		std::wstring key = Trim(line.substr(0, equals));
		std::wstring value = Trim(line.substr(equals + 1));
		if (key.empty() || value.empty()) {
			continue;
		}

		if (_wcsicmp(key.c_str(), L"msg") == 0) {
			rawMessages = value;
			break;
		}
	}

	std::vector<std::wstring> messages;
	size_t begin = 0;
	while (begin <= rawMessages.size()) {
		size_t separator = rawMessages.find(L'|', begin);
		std::wstring item = separator == std::wstring::npos
			? rawMessages.substr(begin)
			: rawMessages.substr(begin, separator - begin);
		item = Trim(item);
		if (!item.empty()) {
			messages.push_back(item);
		}

		if (separator == std::wstring::npos) {
			break;
		}
		begin = separator + 1;
	}

	return messages;
}

void LoadQuickMessages() {
	g_quickMessages.clear();

	std::wstring text;
	if (ReadConfigText(&text)) {
		g_quickMessages = ParseQuickMessages(text);
	}

	Utils::LogFormat("OverlayToolbar quick messages loaded: %d",
		static_cast<int>(g_quickMessages.size()));
}

ToolbarLayout BuildLayout(int surfaceWidth, int surfaceHeight) {
	ToolbarLayout layout = {};
	const int menuCount = g_messageOpen ? std::min(static_cast<int>(g_quickMessages.size()), MENU_MAX_VISIBLE) : 0;
	const int menuHeight = menuCount > 0 ? menuCount * MENU_ITEM_HEIGHT + 4 : 0;
	layout.Width = std::min(760, std::max(320, surfaceWidth - 4));
	layout.Height = TOOLBAR_HEIGHT + menuHeight;
	layout.X = std::max(0, (surfaceWidth - layout.Width) / 2);
	layout.Y = std::max(0, surfaceHeight - layout.Height);
	layout.ToolbarTop = menuHeight;

	const int rowY = layout.ToolbarTop;
	int x = 0;
	auto addButton = [&layout, &x, rowY](int id, int width, const wchar_t* text, bool active) {
		layout.Buttons.push_back({ id, { x, rowY, x + width, rowY + BUTTON_HEIGHT }, text, active });
		x += width + BUTTON_GAP;
	};

	addButton(ButtonToggleTurn, 62,
		Config::isGrandCannonAutoTurnEnabled() ? L"自转:开" : L"自转:关", Config::isGrandCannonAutoTurnEnabled());
	addButton(ButtonToggleInfo, 62,
		Config::isShowEnemyInfo() ? L"敌情:开" : L"敌情:关", Config::isShowEnemyInfo());
	addButton(ButtonScanCannon, 56, L"扫巨炮", true);
	addButton(ButtonPickCrate, 56, L"捡箱", true);

	const wchar_t* routeText = L"跑图捡箱";
	if (IsRouteCrateCaptureActive()) {
		routeText = L"完成区域";
	} else if (IsRouteCrateRunActive()) {
		routeText = L"停止跑图";
	}
	addButton(ButtonRouteCrate, 70, routeText, IsRouteCrateCaptureActive() || IsRouteCrateRunActive());
	addButton(ButtonFormationSquare, 52, L"阵口", true);
	addButton(ButtonFormationVertical, 52, L"阵一", true);
	addButton(ButtonFormationHorizontal, 52, L"阵丨", true);

	int messageLeft = x;
	if (layout.Width - messageLeft < 96) {
		messageLeft = std::max(0, layout.Width - 102);
	}
	layout.MessageRect = { messageLeft, rowY, layout.Width, rowY + BUTTON_HEIGHT };

	for (int i = 0; i < menuCount; ++i) {
		const int itemTop = layout.ToolbarTop - 2 - (menuCount - i) * MENU_ITEM_HEIGHT;
		layout.MessageItems.push_back({ messageLeft, itemTop, layout.Width, itemTop + MENU_ITEM_HEIGHT });
	}

	return layout;
}

void DrawButton(std::vector<unsigned char>* pixels, int width, int height, const ToolbarButton& button) {
	const unsigned char base = button.Active ? 64 : 52;
	const unsigned char green = button.Active ? 92 : 58;
	FillRectRGBA(pixels, width, height, button.Rect, base, green, 66, 236);
	DrawTextRGBA(pixels, width, height, button.Rect, button.Text, true);
}

void DrawToolbar(const ToolbarLayout& layout, std::vector<unsigned char>* pixels) {
	RECT toolbarRect { 0, layout.ToolbarTop, layout.Width, layout.Height };
	FillRectRGBA(pixels, layout.Width, layout.Height, toolbarRect, 26, 28, 27, 232);

	for (const auto& button : layout.Buttons) {
		DrawButton(pixels, layout.Width, layout.Height, button);
	}

	const bool hasMessages = !g_quickMessages.empty();
	FillRectRGBA(pixels, layout.Width, layout.Height, layout.MessageRect, 48, 52, 56, 238);
	DrawTextRGBA(pixels, layout.Width, layout.Height, layout.MessageRect,
		hasMessages ? L"快捷消息" : L"无快捷消息", false);

	if (!g_messageOpen || layout.MessageItems.empty()) {
		return;
	}

	RECT menuRect {
		layout.MessageRect.left,
		layout.MessageItems.front().top,
		layout.MessageRect.right,
		layout.ToolbarTop - 2
	};
	FillRectRGBA(pixels, layout.Width, layout.Height, menuRect, 22, 24, 26, 242);

	for (size_t i = 0; i < layout.MessageItems.size(); ++i) {
		const RECT& itemRect = layout.MessageItems[i];
		if (i % 2 == 0) {
			FillRectRGBA(pixels, layout.Width, layout.Height, itemRect, 34, 37, 40, 220);
		}
		DrawTextRGBA(pixels, layout.Width, layout.Height, itemRect, g_quickMessages[i].c_str(), false);
	}
}

void HandleButtonClick(int id) {
	switch (id) {
	case ButtonToggleTurn: {
		const bool enabled = !Config::isGrandCannonAutoTurnEnabled();
		Config::setGrandCannonAutoTurnEnabled(enabled);
		Utils::LogFormat("OverlayToolbar click: AutoTurn=%d", enabled ? 1 : 0);
		break;
	}
	case ButtonToggleInfo: {
		const bool enabled = !Config::isShowEnemyInfo();
		Config::setShowEnemyInfo(enabled);
		Utils::LogFormat("OverlayToolbar click: ShowEnemyInfo=%d", enabled ? 1 : 0);
		break;
	}
	case ButtonScanCannon:
		Utils::Log("OverlayToolbar click: ScanGrandCannon");
		ForceGrandCannonScan();
		break;
	case ButtonPickCrate:
		Utils::Log("OverlayToolbar click: PickCrate");
		StartOptimalCratePickup();
		break;
	case ButtonRouteCrate:
		Utils::Log("OverlayToolbar click: RouteCrate");
		ToggleRouteCratePickup();
		break;
	case ButtonFormationSquare:
		Utils::Log("OverlayToolbar click: FormationSquare");
		ApplySelectedFormationSquare();
		break;
	case ButtonFormationVertical:
		Utils::Log("OverlayToolbar click: FormationVertical");
		ApplySelectedFormationVertical();
		break;
	case ButtonFormationHorizontal:
		Utils::Log("OverlayToolbar click: FormationHorizontal");
		ApplySelectedFormationHorizontal();
		break;
	default:
		break;
	}
}

bool PollClientCursor(POINT* outPoint, bool* outDown) {
	POINT cursor = {};
	if (!GetCursorPos(&cursor)) {
		return false;
	}

	HWND gameHwnd = GetMainWindowForProcessId(GetCurrentProcessId());
	if (!gameHwnd || !ScreenToClient(gameHwnd, &cursor)) {
		return false;
	}

	*outPoint = cursor;
	*outDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	return true;
}

void HandleMouse(const ToolbarLayout& layout) {
	POINT point = {};
	bool down = false;
	if (!PollClientCursor(&point, &down)) {
		g_mouseWasDown = false;
		return;
	}

	const bool clicked = g_mouseWasDown && !down;
	g_mouseWasDown = down;
	if (!clicked) {
		return;
	}

	const DWORD now = GetTickCount();
	if (now - g_lastClickTick < 120) {
		return;
	}

	const int x = point.x - layout.X;
	const int y = point.y - layout.Y;
	if (x < 0 || y < 0 || x >= layout.Width || y >= layout.Height) {
		g_messageOpen = false;
		return;
	}

	g_lastClickTick = now;

	if (g_messageOpen) {
		for (size_t i = 0; i < layout.MessageItems.size(); ++i) {
			if (ContainsPoint(layout.MessageItems[i], x, y) && i < g_quickMessages.size()) {
				const std::wstring message = g_quickMessages[i];
				g_messageOpen = false;
				Chat(message.c_str());
				return;
			}
		}
	}

	if (ContainsPoint(layout.MessageRect, x, y)) {
		if (g_quickMessages.empty()) {
			LoadQuickMessages();
		}
		g_messageOpen = !g_messageOpen && !g_quickMessages.empty();
		return;
	}

	g_messageOpen = false;
	for (const auto& button : layout.Buttons) {
		if (ContainsPoint(button.Rect, x, y)) {
			HandleButtonClick(button.Id);
			return;
		}
	}
}

void WriteJump(DWORD address, void* target) {
	DWORD oldProtect = 0;
	if (!VirtualProtect(reinterpret_cast<void*>(address), 5, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		Utils::Log("OverlayToolbar hook VirtualProtect failed.");
		return;
	}

	auto* patch = reinterpret_cast<unsigned char*>(address);
	patch[0] = 0xE9;
	*reinterpret_cast<DWORD*>(patch + 1) =
		reinterpret_cast<DWORD>(target) - address - 5;
	VirtualProtect(reinterpret_cast<void*>(address), 5, oldProtect, &oldProtect);
	FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(address), 5);
}

void RenderOverlayToolbarFrameUnsafe() {
	if (!g_enabled) {
		return;
	}

	DSurface* surface = DSurface::Composite;
	if (!surface) {
		return;
	}

	const int surfaceWidth = surface->GetWidth();
	const int surfaceHeight = surface->GetHeight();
	if (surfaceWidth <= 0 || surfaceHeight <= TOOLBAR_HEIGHT) {
		return;
	}

	DrawEnemyInfoOverlay();

	if (!Config::isCustomToolbarEnabled()) {
		return;
	}

	const ToolbarLayout layout = BuildLayout(surfaceWidth, surfaceHeight);
	std::vector<unsigned char> pixels(static_cast<size_t>(layout.Width * layout.Height * 4), 0);
	DrawToolbar(layout, &pixels);
	DrawOverlayRGBA(layout.X, layout.Y, layout.Width, layout.Height, pixels.data());
	HandleMouse(layout);
}

void __stdcall RenderOverlayToolbarFrame() {
	__try {
		RenderOverlayToolbarFrameUnsafe();
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
}

void __declspec(naked) OverlayToolbarDrawHook() {
	__asm {
		pushad
		call RenderOverlayToolbarFrame
		popad
		mov eax, 0x00A8B8B4
		mov eax, [eax]
		jmp dword ptr [g_hookReturn]
	}
}

void EnsureHookInstalled() {
	if (g_hookInstalled) {
		return;
	}

	WriteJump(HOOK_POINT, OverlayToolbarDrawHook);
	g_hookInstalled = true;
	Utils::Log("OverlayToolbar draw hook installed.");
}

}

void InitOverlayToolbar() {
	LoadQuickMessages();
	//EnsureHookInstalled();
	g_enabled = false;
	g_messageOpen = false;
	Utils::Log("InitOverlayToolbar skipped: draw hook disabled.");
}

void UninitOverlayToolbar() {
	g_enabled = false;
	g_messageOpen = false;
	g_mouseWasDown = false;
	Utils::Log("UninitOverlayToolbar!");
}
