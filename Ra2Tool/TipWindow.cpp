#include <windows.h>
#include <process.h>
#include <ShellScalingAPI.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"
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
HBRUSH hBrushTransparent = NULL;

// 强制置顶
void Topmost(HWND hwnd) {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

// 强制刷新
void RefreshLabels(HWND hwnd) {
    ::InvalidateRect(hwnd, NULL, TRUE);
}

// 窗口过程函数
LRESULT CALLBACK TipWindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER:
        if (wParam == TIMER_ID_SHOWTIP) {
            RefreshLabels(hwnd); // 强制刷新
        } else if (wParam == TIMER_ID_TOPMOST) {
            Topmost(hwnd);
        }
        break;
    case WM_PAINT: {
        static COLORREF clr = textColor;
        clr ^= 0x00FFFFFF;;     // 变幻颜色

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 1. 清空整个窗口背景为透明色（必须使用 color key 一致的颜色）
        FillRect(hdc, &gameClientRect, hBrushTransparent);

        // 2. 设置文字透明背景
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, clr);

        // 3. 绘制所有 crate 标签
        ShowCrateInfo(hdc);

        EndPaint(hwnd, &ps);
    }
    break;
    case WM_ERASEBKGND: {
        return 1;               // 禁用背景擦除，防止黑屏
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
        PostQuitMessage(0);
        break;
    case WM_CREATE: {
        // 鼠标穿透要带WS_EX_TRANSPARENT
        LONG winLong = GetWindowLong(hwnd, GWL_EXSTYLE);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, winLong | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
        SetLayeredWindowAttributes(hwnd, maskColor, 0, LWA_COLORKEY);
        SetTimer(hwnd, TIMER_ID_SHOWTIP, 300, NULL);
        SetTimer(hwnd, TIMER_ID_TOPMOST, 2000, NULL);
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
    //HWND hGameMain = FindWindowW(L"Yuris Revenge", L"Yuris Revenge);
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
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,// 扩展样式
        className,                                          // 窗口类名
        className,                                          // 窗口标题
        WS_POPUP | WS_VISIBLE,                              // 窗口样式
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
    g_hwndTipWindow = hwnd;

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
    hBrushTransparent = CreateSolidBrush(maskColor); // 透明色

	// 禁用 DPI 缩放，因红警老游戏不支持，所以咱也禁用，否则坐标会有错位。
    SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE); // Windows 8.1+

    _beginthreadex(
        NULL, 0, ThreadProcCreateTipWindow, NULL, 0, NULL);
}

// 注销所有注册的热键、销毁窗口
void UnInitTipWindow() {
    DeleteObject(hBrushTransparent);
    HWND hwnd = g_hwndTipWindow;
    KillTimer(hwnd, TIMER_ID_SHOWTIP);
    KillTimer(hwnd, TIMER_ID_TOPMOST);
    PostMessage(hwnd, WM_CLOSE, 0, 0);
    g_hwndTipWindow = NULL;
    Utils::Log("UnInitTipWindow!");
}
