#include <windows.h>
#include <vector>
#include <process.h>
#include <ShellScalingAPI.h>
#include "TipWindow.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"
#include "Crate.h"
#include "Utils.h"
#include "Config.h"
#pragma comment(lib, "Shcore.lib")

// 定时器 ID
#define TIMER_ID_SHOWTIP    1
#define TIMER_ID_TOPMOST    2


const COLORREF maskColor = RGB(0, 0, 0);	// 透明颜色
const COLORREF textColor = RGB(255, 0, 0);	// 标签文本颜色


// 保存创建的窗口句柄
HWND g_hwndTipWindow = NULL;
RECT gameClientRect;                        // 游戏客户端矩形区域
POINT gameClientTopLeft = { 0, 0 };         // 游戏客户端左上角坐标
std::vector<HWND> g_crateLabels;
HWND g_crateArrowLabels[8];                 // 8个方向的标签

// 强制置顶
void Topmost(HWND hwnd) {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

HWND createCrateLabel(HWND hWndParent, LPCWSTR lpWindowName, int posX, int posY, int width, int height) {
    HWND hwnd = CreateWindowExW(
        WS_EX_TRANSPARENT,
        L"STATIC",
        lpWindowName,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        posX, posY, width, height,
        hWndParent,
        NULL,
        g_thisModule,
        NULL
    );
	return hwnd;
}

// 窗口过程函数
LRESULT CALLBACK TipWindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER:
        if (wParam == TIMER_ID_SHOWTIP) {
            ShowCrateInfo(hwnd, g_crateLabels);
        } else if (wParam == TIMER_ID_TOPMOST) {
            Topmost(hwnd);
        }
        break;
    case WM_CTLCOLORSTATIC: {
		static COLORREF clr = textColor;
		clr ^= 0x00FFFFFF;; // 变幻颜色
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, clr);
        SetBkMode(hdcStatic, TRANSPARENT);

        // 返回透明背景的刷子（避免填充颜色）
        return (LRESULT)GetStockObject(HOLLOW_BRUSH);   // 返回空刷子
    }
    break;
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH hBrush = CreateSolidBrush(maskColor);
        FillRect(hdc, &rc, hBrush);
        DeleteObject(hBrush);
    }
    break;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;
        break;
    case  WM_NCHITTEST:
        return HTTRANSPARENT;  // 鼠标事件直接穿透
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID_SHOWTIP);
        KillTimer(hwnd, TIMER_ID_TOPMOST);
        PostQuitMessage(0);
        g_hwndTipWindow = NULL;
        break;
    case WM_CREATE: {
        // 鼠标穿透要带WS_EX_TRANSPARENT
        LONG lWindLong = GetWindowLong(hwnd, GWL_EXSTYLE);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, lWindLong | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
        SetLayeredWindowAttributes(hwnd, maskColor, 0, LWA_COLORKEY);

        SetTimer(hwnd, TIMER_ID_SHOWTIP, 350, NULL);
        SetTimer(hwnd, TIMER_ID_TOPMOST, 1000, NULL);
    }
    break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// 线程函数
unsigned __stdcall ThreadProcCreateTipWindow(void* param) {
	//::CoInitialize(NULL);
    //HWND hGameMain = FindWindowW(L"RA2", nullptr);
    gameClientTopLeft = { 0, 0 };   // 需要每次初始化
    HWND hGameMain = GetMainWindowForProcessId(GetCurrentProcessId());
    GetClientRect(hGameMain, &gameClientRect);
    ClientToScreen(hGameMain, &gameClientTopLeft);
    Utils::LogFormat("Game Wnd: %p Client Rect: (%d, %d, %d, %d)", hGameMain,
        gameClientTopLeft.x, gameClientTopLeft.y,
		gameClientRect.right, gameClientRect.bottom);

    const char* className = "RA2TipWindow";
    HWND hwnd = NULL;
    MSG msg = {};
    WNDCLASS wc = {};
    wc.lpfnWndProc = TipWindowWndProc;
    wc.hInstance = g_thisModule;
    wc.lpszClassName = className;
    RegisterClass(&wc);

    // 创建窗口
    hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,  // 扩展样式
        className,                                          // 窗口类名
        className,                                          // 窗口标题
        WS_POPUP,                                           // 窗口样式
        gameClientTopLeft.x, gameClientTopLeft.y,           // 窗口位置
        gameClientRect.right, gameClientRect.bottom,        // 窗口尺寸
        NULL,                                               // 父窗口
        NULL,                                               // 菜单
        wc.hInstance,                                       // 程序实例句柄
        NULL                                                // 附加参数
    );

    if (!hwnd) {
        Utils::Log("Tip Window creation failed!");
        goto _exit;
    }

    // 设置窗口为全透明 & 支持绘图
    g_hwndTipWindow = hwnd;
    SetLayeredWindowAttributes(hwnd, 0, 0, LWA_COLORKEY);
    ShowWindow(hwnd, SW_SHOWNOACTIVATE);

    // 消息循环
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

_exit:
    //::CoUninitialize();
    return 0;
}


void InitTipWindow() {
    Utils::Log("InitTipWindow!");
	g_crateLabels.resize(MAX_CRATE_COUNT, NULL);

	// 禁用 DPI 缩放，因红警老游戏不支持，所以咱也禁用，否则坐标会有错位。
    SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE); // Windows 8.1+

    _beginthreadex(
        NULL, 0, ThreadProcCreateTipWindow, NULL, 0, NULL);
}

// 注销所有注册的热键、销毁窗口
void UnInitTipWindow() {
    for(auto &label : g_crateLabels) {
        if (label) {
            DestroyWindow(label);
            label = NULL;
        }
	}
	g_crateLabels.clear();

    HWND hwnd = g_hwndTipWindow;
    PostMessage(hwnd, WM_CLOSE, 0, 0);
    Utils::Log("UnInitTipWindow!");
}
