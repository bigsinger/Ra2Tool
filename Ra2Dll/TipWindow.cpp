#include <windows.h>
#include <process.h>
#include "TipWindow.h"
#include "Ra2Helper.h"
#include "Crate.h"
#include "Utils.h"
#include "Config.h"


// 定时器 ID
#define TIMER_ID_TEST       1
#define TIMER_ID_TOPMOST    2


// 保存创建的窗口句柄
HWND g_hwndTipWindow = NULL;
int g_nScreenWidth = 0;
int g_nScreenHeight = 0;


// 强制置顶
void Topmost(HWND hwnd) {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

// 窗口过程函数
LRESULT CALLBACK TipWindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER:
        if (wParam == TIMER_ID_TOPMOST) {
            Topmost(hwnd);
        } else if (wParam == TIMER_ID_TEST) {
            ShowCrateInfo(g_hwndTipWindow);
        }
        break;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;
    case WM_CLOSE:
    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID_TEST);
        KillTimer(hwnd, TIMER_ID_TOPMOST);
        PostQuitMessage(0);
        break;
    case WM_CREATE:
        SetTimer(hwnd, TIMER_ID_TEST, 3000, NULL);
        SetTimer(hwnd, TIMER_ID_TOPMOST, 1000, NULL);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// 线程函数
unsigned __stdcall ThreadProcCreateTipWindow(void* param) {
	//::CoInitialize(NULL);
    g_nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    g_nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    const char* className = "RA2TipWindow";
    HWND hwnd = NULL;
    MSG msg = {};
    WNDCLASS wc = {};
    wc.lpfnWndProc = TipWindowWndProc;
    wc.hInstance = g_thisModule;
    wc.lpszClassName = className;

    if (!RegisterClass(&wc)) {
        Utils::Log("Window class registration failed!");
        goto _exit;
    }



    // 创建窗口
    hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,  // 扩展样式
        className,                                          // 窗口类名
        className,                                          // 窗口标题
        WS_POPUP,                                           // 窗口样式
        0, 0, g_nScreenWidth, g_nScreenHeight,              // 窗口位置和尺寸
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
    _beginthreadex(
        NULL, 0, ThreadProcCreateTipWindow, NULL, 0, NULL);
}

// 注销所有注册的热键、销毁窗口
void UnInitTipWindow() {
    HWND hwnd = g_hwndTipWindow;
    DestroyWindow(hwnd);
    Utils::Log("UnInitTipWindow!");
}
