#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <ShellScalingAPI.h>
#include <vector>
#include <string>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"
#pragma comment(lib, "Shcore.lib")

namespace {

constexpr UINT_PTR TIMER_ID_TOOLBAR_REFRESH = 1;
constexpr UINT_PTR TIMER_ID_TOOLBAR_TOPMOST = 2;
constexpr int TOOLBAR_WIDTH = 624;
constexpr int TOOLBAR_HEIGHT = 40;
constexpr int BUTTON_WIDTH = 82;
constexpr int BUTTON_HEIGHT = 26;
constexpr int BUTTON_Y = 7;
constexpr int COMBO_ID_QUICK_MESSAGE = 1001;
constexpr COLORREF TOOLBAR_BG = RGB(28, 28, 28);

HWND g_hwndCustomToolbar = NULL;
HWND g_hwndQuickMessageCombo = NULL;
std::vector<std::wstring> g_quickMessages;

struct ToolbarButton {
	int Id;
	RECT Rect;
	const wchar_t* Text;
};

enum ToolbarButtonId {
	ButtonToggleTurn = 1,
	ButtonToggleInfo = 2,
	ButtonScanCannon = 3,
	ButtonPickCrate = 4,
	ButtonRouteCrate = 5,
};

ToolbarButton buttons[] = {
	{ ButtonToggleTurn, { 6, BUTTON_Y, 6 + BUTTON_WIDTH, BUTTON_Y + BUTTON_HEIGHT }, L"" },
	{ ButtonToggleInfo, { 92, BUTTON_Y, 92 + BUTTON_WIDTH, BUTTON_Y + BUTTON_HEIGHT }, L"" },
	{ ButtonScanCannon, { 178, BUTTON_Y, 178 + BUTTON_WIDTH, BUTTON_Y + BUTTON_HEIGHT }, L"\x626B\x5DE8\x70AE" },
	{ ButtonPickCrate, { 264, BUTTON_Y, 264 + BUTTON_WIDTH, BUTTON_Y + BUTTON_HEIGHT }, L"\x6361\x7BB1\x5B50" },
	{ ButtonRouteCrate, { 350, BUTTON_Y, 350 + BUTTON_WIDTH, BUTTON_Y + BUTTON_HEIGHT }, L"" },
};

std::wstring AnsiToWide(const char* text) {
	if (!text || !*text) {
		return std::wstring();
	}

	const int needed = MultiByteToWideChar(CP_ACP, 0, text, -1, NULL, 0);
	if (needed <= 1) {
		return std::wstring();
	}

	std::wstring result(static_cast<size_t>(needed - 1), L'\0');
	MultiByteToWideChar(CP_ACP, 0, text, -1, &result[0], needed);
	return result;
}

void LoadQuickMessages() {
	g_quickMessages.clear();

	const char* path = Config::getConfigFilePath();
	int count = ::GetPrivateProfileIntA("quickmessages", "Count", 0, path);
	if (count <= 0) {
		count = 20;
	}

	for (int i = 1; i <= count; ++i) {
		char key[16] = {};
		char value[512] = {};
		sprintf_s(key, "%d", i);
		::GetPrivateProfileStringA("quickmessages", key, "", value, sizeof(value), path);
		if (!value[0]) {
			continue;
		}

		std::wstring wide = AnsiToWide(value);
		if (!wide.empty()) {
			g_quickMessages.push_back(wide);
		}
	}

	Utils::LogFormat("CustomToolbar quick messages loaded: %d", static_cast<int>(g_quickMessages.size()));
}

void PopulateQuickMessageCombo(HWND combo) {
	if (!combo) {
		return;
	}

	SendMessageW(combo, CB_RESETCONTENT, 0, 0);
	SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"\x5FEB\x6377\x6D88\x606F"));

	for (const auto& message : g_quickMessages) {
		SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(message.c_str()));
	}

	SendMessageW(combo, CB_SETCURSEL, 0, 0);
}

void CreateQuickMessageCombo(HWND hwnd) {
	LoadQuickMessages();

	g_hwndQuickMessageCombo = CreateWindowExW(
		0,
		L"COMBOBOX",
		L"",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		438,
		6,
		180,
		180,
		hwnd,
		reinterpret_cast<HMENU>(COMBO_ID_QUICK_MESSAGE),
		g_thisModule,
		NULL);

	PopulateQuickMessageCombo(g_hwndQuickMessageCombo);
}

void UpdateButtonText() {
	buttons[0].Text = Config::isGrandCannonAutoTurnEnabled() ? L"\x81EA\x8F6C:\x5F00" : L"\x81EA\x8F6C:\x5173";
	buttons[1].Text = Config::isShowEnemyInfo() ? L"\x654C\x60C5:\x5F00" : L"\x654C\x60C5:\x5173";

	if (IsRouteCrateCaptureActive()) {
		buttons[4].Text = L"\x5B8C\x6210\x8DEF\x7EBF";
	} else if (IsRouteCrateRunActive()) {
		buttons[4].Text = L"\x505C\x6B62\x8DD1\x56FE";
	} else {
		buttons[4].Text = L"\x8DD1\x56FE\x6361\x7BB1";
	}
}

void Topmost(HWND hwnd) {
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void DrawButton(HDC hdc, const ToolbarButton& button) {
	const bool enabled =
		(button.Id == ButtonToggleTurn && Config::isGrandCannonAutoTurnEnabled())
		|| (button.Id == ButtonToggleInfo && Config::isShowEnemyInfo())
		|| (button.Id == ButtonRouteCrate && (IsRouteCrateCaptureActive() || IsRouteCrateRunActive()))
		|| button.Id == ButtonScanCannon
		|| button.Id == ButtonPickCrate;

	HBRUSH brush = CreateSolidBrush(enabled ? RGB(58, 84, 66) : RGB(58, 58, 58));
	FillRect(hdc, &button.Rect, brush);
	DeleteObject(brush);

	HBRUSH frame = CreateSolidBrush(RGB(150, 150, 150));
	FrameRect(hdc, &button.Rect, frame);
	DeleteObject(frame);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(238, 238, 238));
	DrawTextW(hdc, button.Text, -1, const_cast<RECT*>(&button.Rect), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void DrawToolbar(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	RECT rect = { 0, 0, TOOLBAR_WIDTH, TOOLBAR_HEIGHT };
	HBRUSH bg = CreateSolidBrush(TOOLBAR_BG);
	FillRect(hdc, &rect, bg);
	DeleteObject(bg);

	HBRUSH frame = CreateSolidBrush(RGB(90, 90, 90));
	FrameRect(hdc, &rect, frame);
	DeleteObject(frame);

	UpdateButtonText();
	for (const auto& button : buttons) {
		DrawButton(hdc, button);
	}

	EndPaint(hwnd, &ps);
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
	default:
		break;
	}

	InvalidateRect(hwnd, NULL, TRUE);
	if (g_hwndTipWindow) {
		InvalidateRect(g_hwndTipWindow, NULL, TRUE);
	}
}

void HandleQuickMessageSelection() {
	if (!g_hwndQuickMessageCombo) {
		return;
	}

	const int index = static_cast<int>(SendMessageW(g_hwndQuickMessageCombo, CB_GETCURSEL, 0, 0));
	if (index <= 0 || index > static_cast<int>(g_quickMessages.size())) {
		return;
	}

	const std::wstring& message = g_quickMessages[static_cast<size_t>(index - 1)];
	SendQuickGlobalMessage(message.c_str());
	SendMessageW(g_hwndQuickMessageCombo, CB_SETCURSEL, 0, 0);
}

int HitTestButton(int x, int y) {
	POINT pt = { x, y };
	for (const auto& button : buttons) {
		if (PtInRect(&button.Rect, pt)) {
			return button.Id;
		}
	}
	return 0;
}

LRESULT CALLBACK CustomToolbarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		CreateQuickMessageCombo(hwnd);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == COMBO_ID_QUICK_MESSAGE && HIWORD(wParam) == CBN_SELCHANGE) {
			HandleQuickMessageSelection();
		}
		break;
	case WM_TIMER:
		if (wParam == TIMER_ID_TOOLBAR_REFRESH) {
			InvalidateRect(hwnd, NULL, TRUE);
		} else if (wParam == TIMER_ID_TOOLBAR_TOPMOST) {
			Topmost(hwnd);
		}
		break;
	case WM_PAINT:
		DrawToolbar(hwnd);
		break;
	case WM_LBUTTONUP: {
		const int x = GET_X_LPARAM(lParam);
		const int y = GET_Y_LPARAM(lParam);
		const int id = HitTestButton(x, y);
		if (id) {
			HandleButtonClick(id, hwnd);
		}
		break;
	}
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	case WM_ERASEBKGND:
		return 1;
	case WM_CLOSE:
	case WM_DESTROY:
		g_hwndQuickMessageCombo = NULL;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

unsigned __stdcall ThreadProcCreateCustomToolbar(void* param) {
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);

	RECT clientRect = {};
	POINT topLeft = { 0, 0 };
	HWND gameHwnd = GetMainWindowForProcessId(GetCurrentProcessId());
	if (!gameHwnd || !GetClientRect(gameHwnd, &clientRect)) {
		Utils::Log("CustomToolbar cannot find game window.");
		return 0;
	}
	ClientToScreen(gameHwnd, &topLeft);

	const int width = clientRect.right - clientRect.left;
	const int height = clientRect.bottom - clientRect.top;
	const int toolbarWidth = min(TOOLBAR_WIDTH, max(320, width - 16));
	const int x = topLeft.x + max(8, (width - toolbarWidth) / 2);
	const int y = topLeft.y + max(8, height - TOOLBAR_HEIGHT - 5);

	const char* className = "RA2CustomToolbar";
	MSG msg = {};
	WNDCLASS wc = {};
	wc.lpfnWndProc = CustomToolbarWndProc;
	wc.hInstance = g_thisModule;
	wc.lpszClassName = className;
	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
		className,
		className,
		WS_POPUP | WS_VISIBLE,
		x,
		y,
		toolbarWidth,
		TOOLBAR_HEIGHT,
		NULL,
		NULL,
		wc.hInstance,
		NULL);

	if (!hwnd) {
		Utils::Log("CustomToolbar window creation failed.");
		return 0;
	}

	g_hwndCustomToolbar = hwnd;
	SetLayeredWindowAttributes(hwnd, 0, 235, LWA_ALPHA);
	SetTimer(hwnd, TIMER_ID_TOOLBAR_REFRESH, 500, NULL);
	SetTimer(hwnd, TIMER_ID_TOOLBAR_TOPMOST, 2000, NULL);

	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

}

void InitCustomToolbar() {
	if (g_hwndCustomToolbar || !Config::isCustomToolbarEnabled()) {
		return;
	}

	Utils::Log("InitCustomToolbar!");
	_beginthreadex(NULL, 0, ThreadProcCreateCustomToolbar, NULL, 0, NULL);
}

void UnInitCustomToolbar() {
	HWND hwnd = g_hwndCustomToolbar;
	if (!hwnd) {
		return;
	}

	KillTimer(hwnd, TIMER_ID_TOOLBAR_REFRESH);
	KillTimer(hwnd, TIMER_ID_TOOLBAR_TOPMOST);
	PostMessage(hwnd, WM_CLOSE, 0, 0);
	g_hwndCustomToolbar = NULL;
	Utils::Log("UnInitCustomToolbar!");
}
