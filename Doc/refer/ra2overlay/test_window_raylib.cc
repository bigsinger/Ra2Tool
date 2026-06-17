#include "windows_shit.h"
#define EAT_SHIT_FIRST  // prevent linter move windows shit down

#include "logging.h"
#include "raylibpp.h"
#include "window.h"

using namespace ra2overlay;

struct PlatformData {
  int screen_width;
  int screen_height;
  HWND hwnd;        // Window handler
  HDC hdc;          // Graphic context handler
  // Software renderer variables
  HDC hdcmem;            // Memory graphic context handler
  HBITMAP hbitmap;       // GDI bitmap handler
  unsigned int* pixels;  // Pointer to pixel data buffer (BGRA format)
};

static PlatformData platform{0};
static std::unique_ptr<gui::Window> g_window;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_SIZE:
      if (wParam == SIZE_MINIMIZED) return 0;
      return 0;
    case WM_SYSCOMMAND:
      if ((wParam & 0xfff0) == SC_KEYMENU)  // Disable ALT application menu
        return 0;
      break;
    case WM_CLOSE:
      g_window->Close();
      break;
    case WM_DESTROY:
      if (platform.hdcmem) {
        DeleteDC(platform.hdcmem);
        platform.hdcmem = NULL;
      }
      if (platform.hbitmap) {
        DeleteObject(platform.hbitmap);  // Clears platform.pixels data
        platform.hbitmap = NULL;
        platform.pixels = NULL;  // NOTE: Pointer invalid after DeleteObject()
      }
      ::PostQuitMessage(0);
      break;
    case WM_DPICHANGED:
      break;
    case WM_PAINT: {
      PAINTSTRUCT ps = {0};
      HDC hdc = BeginPaint(hWnd, &ps);
      // Blit from memory DC to window DC
      BitBlt(hdc, 0, 0, platform.screen_width, platform.screen_height,
             platform.hdcmem, 0, 0, SRCCOPY);
      EndPaint(hWnd, &ps);
    }
  }
  return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

// Main code
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow) {
  InitLogging(LogTarget::kStandard);
  // Load configurations.
  char exe_path[MAX_PATH] = {0};
  DWORD nSize = GetModuleFileNameA(hInstance, exe_path, _countof(exe_path));
  if (nSize == 0) {
    LOG_F(FATAL, "Failed to get module file name {}", GetLastError());
  }
  g_window = std::make_unique<gui::Window>();
  gui::Window* window = g_window.get();

  gui::Window::Params params{
      .name = "Test window",
      .width = 1280,
      .height = 800,
      .with_border = true,
      .hInstance = hInstance,
      .nShowCmd = nCmdShow,
      .wndproc = WndProc,
      .hIcon = NULL,
      .hCursor = NULL,
  };
  // Create application window
  WNDCLASSA WndClass;
  WndClass.style = 3;
  WndClass.lpfnWndProc = params.wndproc;
  WndClass.cbClsExtra = 0;
  WndClass.cbWndExtra = 0;
  WndClass.hInstance = params.hInstance;
  WndClass.hIcon = params.hIcon;
  WndClass.hCursor = params.hCursor;
  WndClass.hbrBackground = 0;
  WndClass.lpszMenuName = 0;
  WndClass.lpszClassName = params.name.c_str();
  RegisterClassA(&WndClass);
  HWND hwnd;
  if (params.with_border) {
    hwnd = CreateWindowExA(0, params.name.c_str(), params.name.c_str(),
                           0x00CA0000u, 0, 0, 0, 0, 0, 0, params.hInstance, 0);
  } else {
    hwnd = CreateWindowExA(0, params.name.c_str(), params.name.c_str(),
                           0x860A0000, 0, 0, 0, 0, 0, 0, params.hInstance, 0);
  }
  CHECK(hwnd != NULL);
  gui::InitWindow(params.width, params.height, params.name.c_str(),
                  reinterpret_cast<void*>(hwnd));
  RECT rc;
  SetRect(&rc, 0, 0, params.width, params.height);
  DWORD style = GetWindowLongA(hwnd, GWL_STYLE);
  DWORD exStyle = GetWindowLongA(hwnd, GWL_EXSTYLE);
  BOOL hasMenu = GetMenu(hwnd) != 0;
  CHECK(AdjustWindowRectEx(&rc, style, hasMenu, exStyle));
  CHECK(MoveWindow(hwnd, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                   /*bRepaint=*/TRUE));
  // Show the window
  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);
  // Initialize software framebuffer
  platform.screen_width = params.width;
  platform.screen_height = params.height;
  platform.hwnd = hwnd;
  platform.hdc = GetDC(platform.hwnd);
  BITMAPINFO bmi = {0};
  ZeroMemory(&bmi, sizeof(bmi));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = params.width;
  bmi.bmiHeader.biHeight = -(int)(params.height);  // Top-down bitmap
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;  // 32-bit BGRA
  bmi.bmiHeader.biCompression = BI_RGB;
  platform.hdcmem = CreateCompatibleDC(platform.hdc);
  platform.hbitmap = CreateDIBSection(platform.hdcmem, &bmi, DIB_RGB_COLORS,
                                      (void**)&platform.pixels, NULL, 0);
  SelectObject(platform.hdcmem, platform.hbitmap);

  window->InitPannel(params.width, params.height);
  std::vector<gui::ToolDesc> tools{
      gui::ToolDesc{.id = 4,
                    .desc = "Tool 1",
                    .cb_active = [](gui::ToolId, bool) {},
                    .cb_reload = [](gui::ToolId) {}},
      gui::ToolDesc{.id = 3,
                    .desc = "Tool 2",
                    .cb_active = [](gui::ToolId, bool) {},
                    .cb_reload = [](gui::ToolId) {}},
  };
  window->UpdateTools(tools);

  // Main loop
  while (window->is_running()) {
    window->ClearScreen();
    window->Render();
    gui::Window::SWFramebuffer fb = window->GetFramebuffer();
    memcpy(platform.pixels, fb.buf.data(), fb.buf.size());
    // Force redraw
    if (!platform.hdc) abort();
    InvalidateRect(platform.hwnd, NULL, FALSE);
    UpdateWindow(platform.hwnd);
    // Poll input events before next update.
    window->PollEvents();
  }

  // Cleanup
  g_window.reset();
  return 0;
}
