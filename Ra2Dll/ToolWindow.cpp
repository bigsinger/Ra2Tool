#include <windows.h>
#include <process.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"


// 快捷键 ID
#define HOTKEY_ALT_R 1
#define HOTKEY_ALT_M 2
#define HOTKEY_ALT_P 3
#define HOTKEY_ALT_B 4
#define HOTKEY_ALT_G 5
#define HOTKEY_ALT_L 6
#define HOTKEY_ALT_C 7

// 定时器 ID
#define TIMER_ID_AutoRepair             1
#define TIMER_ID_OpenPsychicDetection   2

// 处理函数声明
void OnAltR();
void OnAltM();
void OnAltP();
void OnAltB();
void OnAltG();
void OnAltL();
void OnAltC();


// 保存创建的窗口句柄
HWND g_hwndToolWindow = NULL;

// 窗口过程函数
LRESULT CALLBACK ToolWindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER:
        if (wParam == TIMER_ID_AutoRepair) {
            AutoRepair();
        } else if (wParam == TIMER_ID_OpenPsychicDetection) {
			static int OpenPsychicDetectionCount = 0;
            if (++OpenPsychicDetectionCount < 5) {
                OpenPsychicDetection();
            }
        }
        break;
    case WM_HOTKEY:
        switch (wParam) {
        case HOTKEY_ALT_R: OnAltR(); break;
        case HOTKEY_ALT_M: OnAltM(); break;
        case HOTKEY_ALT_P: OnAltP(); break;
        case HOTKEY_ALT_B: OnAltB(); break;
        case HOTKEY_ALT_G: OnAltG(); break;
        case HOTKEY_ALT_L: OnAltL(); break;
        case HOTKEY_ALT_C: OnAltC(); break;
        }
        break;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;
    case WM_CLOSE:
    case WM_DESTROY:
        UnInitTipWindow();
        PostQuitMessage(0);
        break;
    case WM_CREATE:
        if (Config::isAutoShowCrate()) {
            InitTipWindow();
        }
        if(Config::isAutoRepair()){
            SetTimer(hwnd, TIMER_ID_AutoRepair, 1000, NULL);
		}
        if (Config::isAutoOpenPsychicDetection()) {
            SetTimer(hwnd, TIMER_ID_OpenPsychicDetection, 10000, NULL);
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// 处理 ALT + R 快捷键
void OnAltR() {
    Utils::Log("OnAltR!");
    AutoRepair();           // 自动修理
}

// 处理 ALT + M 快捷键
void OnAltM() {
    Utils::Log("OnAltM!");
    GiveMeMoney();          // 钱多多
}

// 处理 ALT + P 快捷键
void OnAltP() {
    Utils::Log("OnAltP!");
    OpenPsychicDetection(); // 心灵感应
}

// 处理 ALT + B 快捷键
void OnAltB() {
    Utils::Log("OnAltB!");
    ClearBeacons();         // 清除信标
}

// 处理 ALT + G 快捷键
void OnAltG() {
    Utils::Log("OnAltG!");
    OpenTech();             // 科技全开
}

// 处理 ALT + L 快捷键
void OnAltL() {
    Utils::Log("OnAltL!");
    LevelUpSelectings();    // 升级选中单位等级
}

// 处理 ALT + C 快捷键
void OnAltC() {
	Utils::Log("OnAltC!");
    if (g_hwndTipWindow) {
		UnInitTipWindow();
    } else {
        InitTipWindow();
    }
}

// 线程函数
unsigned __stdcall ThreadProcCreateToolWindow(void* param) {
	//::CoInitialize(NULL);
    const char* className = "RA2ToolWindow";
    HWND hwnd = NULL;
    MSG msg = {};
    WNDCLASS wc = {};
    wc.lpfnWndProc = ToolWindowWndProc;
    wc.hInstance = g_thisModule;
    wc.lpszClassName = className;
    RegisterClass(&wc);

    // 创建窗口
    hwnd = CreateWindowEx(
        WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,// 扩展样式
        className,                          // 窗口类名
        className,                          // 窗口标题
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,   // 窗口样式
        -900, -900, 0, 0,                   // 窗口位置和尺寸
        NULL,                               // 父窗口
        nullptr,                            // 菜单
        wc.hInstance,                       // 程序实例句柄
        nullptr                             // 附加参数
    );

    if (!hwnd) {
        Utils::Log("Window creation failed!");
        goto _exit;
    }
    g_hwndToolWindow = hwnd;

    // 注册全局快捷键
    if (!RegisterHotKey(hwnd, HOTKEY_ALT_R, MOD_ALT, 'R')) {
        Utils::Log("Register ALT + R hotkey Failed!");
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_M, MOD_ALT, 'M')) {
        Utils::Log("Register ALT + M hotkey Failed!");
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_P, MOD_ALT, 'P')) {
        Utils::Log("Register ALT + P hotkey Failed!");
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_B, MOD_ALT, 'B')) {
        Utils::Log("Register ALT + B hotkey Failed!");
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_G, MOD_ALT, 'G')) {
        Utils::Log("Register ALT + G hotkey Failed!");
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_L, MOD_ALT, 'L')) {
        Utils::Log("Register ALT + L hotkey Failed!");
    }

    if (!RegisterHotKey(hwnd, HOTKEY_ALT_C, MOD_ALT, 'C')) {
        Utils::Log("Register ALT + C hotkey Failed!");
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
    Utils::Log("InitToolWindow!");
    _beginthreadex(
        NULL, 0, ThreadProcCreateToolWindow, NULL, 0, NULL);
}

// 注销所有注册的热键、销毁窗口
void UnInitToolWindow() {
    HWND hwnd = g_hwndToolWindow;
    KillTimer(hwnd, TIMER_ID_AutoRepair);
    KillTimer(hwnd, TIMER_ID_OpenPsychicDetection);

    UnregisterHotKey(hwnd, HOTKEY_ALT_R);
    UnregisterHotKey(hwnd, HOTKEY_ALT_M);
    UnregisterHotKey(hwnd, HOTKEY_ALT_P);
    UnregisterHotKey(hwnd, HOTKEY_ALT_B);
    UnregisterHotKey(hwnd, HOTKEY_ALT_G);
    UnregisterHotKey(hwnd, HOTKEY_ALT_L);
    UnregisterHotKey(hwnd, HOTKEY_ALT_C);

    DestroyWindow(hwnd);
    Utils::Log("UnInitToolWindow!");
}
