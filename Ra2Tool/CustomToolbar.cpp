#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <ShellScalingAPI.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"
#pragma comment(lib, "Shcore.lib")

namespace {

constexpr UINT_PTR TIMER_ID_TOOLBAR_REFRESH = 1;
constexpr UINT_PTR TIMER_ID_TOOLBAR_TOPMOST = 2;
constexpr int TOOLBAR_WIDTH = 330;
constexpr int TOOLBAR_HEIGHT = 38;
constexpr int BUTTON_WIDTH = 96;
constexpr int BUTTON_HEIGHT = 26;
constexpr COLORREF TOOLBAR_BG = RGB(28, 28, 28);

HWND g_hwndCustomToolbar = NULL;

struct ToolbarButton {
	int Id;
	RECT Rect;
	const wchar_t* Text;
};

enum ToolbarButtonId {
	ButtonToggleTurn = 1,
	ButtonToggleInfo = 2,
	ButtonScanCannon = 3,
};

ToolbarButton buttons[] = {
	{ ButtonToggleTurn, { 8, 6, 8 + BUTTON_WIDTH, 6 + BUTTON_HEIGHT }, L"" },
	{ ButtonToggleInfo, { 116, 6, 116 + BUTTON_WIDTH, 6 + BUTTON_HEIGHT }, L"" },
	{ ButtonScanCannon, { 224, 6, 224 + BUTTON_WIDTH, 6 + BUTTON_HEIGHT }, L"\x626B\x63CF\x5DE8\x70AE" },
};

void UpdateButtonText() {
	buttons[0].Text = Config::isGrandCannonAutoTurnEnabled() ? L"\x81EA\x8F6C:\x5F00" : L"\x81EA\x8F6C:\x5173";
	buttons[1].Text = Config::isShowEnemyInfo() ? L"\x654C\x60C5:\x5F00" : L"\x654C\x60C5:\x5173";
}

void Topmost(HWND hwnd) {
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void DrawButton(HDC hdc, const ToolbarButton& button) {
	const bool enabled =
		(button.Id == ButtonToggleTurn && Config::isGrandCannonAutoTurnEnabled())
		|| (button.Id == ButtonToggleInfo && Config::isShowEnemyInfo())
		|| button.Id == ButtonScanCannon;

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
	default:
		break;
	}

	InvalidateRect(hwnd, NULL, TRUE);
	if (g_hwndTipWindow) {
		InvalidateRect(g_hwndTipWindow, NULL, TRUE);
	}
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
	const int x = topLeft.x + max(8, (width - TOOLBAR_WIDTH) / 2);
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
		TOOLBAR_WIDTH,
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
