#include <windows.h>
#include <process.h>
#include "Utils.h"
#include "Ra2Helper.h"
#include "ToolWindow.h"
#include "AutoRepair.h"


// 快捷键 ID
#define HOTKEY_ALT_R 1
#define HOTKEY_ALT_M 2
#define HOTKEY_ALT_P 3
#define HOTKEY_ALT_B 4
#define HOTKEY_ALT_G 5

// 处理函数声明
void OnAltR();
void OnAltM();
void OnAltP();
void OnAltB();
void OnAltG();


// 保存创建的窗口句柄
HWND g_hwndToolWindow = NULL;

// 窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_HOTKEY:
        switch (wParam) {
        case HOTKEY_ALT_R: OnAltR(); break;
        case HOTKEY_ALT_M: OnAltM(); break;
        case HOTKEY_ALT_P: OnAltP(); break;
        case HOTKEY_ALT_B: OnAltB(); break;
        case HOTKEY_ALT_G: OnAltG(); break;
        }
        break;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// 处理 ALT + R 快捷键
void OnAltR() {
	AutoRepair();           // 自动修理
}

// 处理 ALT + M 快捷键
void OnAltM() {
    GiveMeMoney();          // 钱多多
}

// 处理 ALT + P 快捷键
void OnAltP() {
	OpenPsychicDetection(); // 心灵感应
}

// 处理 ALT + B 快捷键
void OnAltB() {
	ClearBeacons();         // 清除信标
}

// 处理 ALT + G 快捷键
void OnAltG() {
	OpenTech();             // 科技全开
}

// 线程函数
unsigned __stdcall ThreadProcCreateToolWindow(void* param) {
	//::CoInitialize(NULL);
    const char* className = "RA2ToolWindow";
    HWND hwnd = NULL;
    MSG msg = {};
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = g_thisModule;
    wc.lpszClassName = className;

    if (!RegisterClass(&wc)) {
        Utils::Log("Window class registration failed!");
        goto _exit;
    }

    // 创建窗口
    hwnd = CreateWindowEx(
        WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,// 扩展样式
        className,                      // 窗口类名
        className,                      // 窗口标题
        WS_OVERLAPPEDWINDOW,            // 窗口样式
        -900, -900, 0, 0,               // 窗口位置和尺寸
        NULL,                           // 父窗口
        nullptr,                        // 菜单
        wc.hInstance,                   // 程序实例句柄
        nullptr                         // 附加参数
    );

    if (!hwnd) {
        Utils::Log("Window creation failed!");
        goto _exit;
    }

    // 隐藏窗口
    ShowWindow(hwnd, SW_SHOW);
    g_hwndToolWindow = hwnd;

    // 注册全局快捷键
    if (!RegisterHotKey(hwnd, HOTKEY_ALT_R, MOD_ALT, 'R')) {
        Utils::Log("Register ALT + R hotkey Failed!");
        goto _exit;
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_M, MOD_ALT, 'M')) {
        Utils::Log("Register ALT + M hotkey Failed!");
        goto _exit;
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_P, MOD_ALT, 'P')) {
        Utils::Log("Register ALT + P hotkey Failed!");
        goto _exit;
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_B, MOD_ALT, 'B')) {
        Utils::Log("Register ALT + B hotkey Failed!");
        goto _exit;
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_G, MOD_ALT, 'G')) {
        Utils::Log("Register ALT + G hotkey Failed!");
		goto _exit;
    }

    // 消息循环
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

_exit:
    //::CoUninitialize();
    return 0;
}


void InitToolWindow() {
    // 创建线程
    _beginthreadex(
        NULL, 0, ThreadProcCreateToolWindow, NULL, 0, NULL);
}

// 注销所有注册的热键、销毁窗口
void UnInitToolWindow() {
    HWND hwnd = g_hwndToolWindow;

    UnregisterHotKey(hwnd, HOTKEY_ALT_R);
    UnregisterHotKey(hwnd, HOTKEY_ALT_M);
    UnregisterHotKey(hwnd, HOTKEY_ALT_P);
    UnregisterHotKey(hwnd, HOTKEY_ALT_B);
    UnregisterHotKey(hwnd, HOTKEY_ALT_G);

    DestroyWindow(hwnd);
    Utils::Log("Destroy ToolWindow!");
}
