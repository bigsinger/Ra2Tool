#include "base/macro.h"
__ZEUS_BEGIN_THIRD_PARTY_HEADERS
#include "FileSystem.h"
#include "ShapeButtonClass.h"
#include "Syringe.h"
#include "Unsorted.h"
__ZEUS_END_THIRD_PARTY_HEADERS
#include "Utilities/Log.h"

#include <Windows.h>
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace zeus {
namespace loading {

static std::vector<ShapeButtonClass*> g_btns;
static HHOOK g_wnd_proc_hook = nullptr;

static void CheckCommandButtons(uint32_t frame) {
  for (ShapeButtonClass* btn : g_btns) {
    if (btn->IsPressed) {
      Log::WriteLine("[%d] Button pressed", frame);
    }
  }
}

static LRESULT CALLBACK CommandButtonWndProcHook(int code, WPARAM wParam,
                                                 LPARAM lParam) {
  if (code >= 0) {
    const auto* msg = reinterpret_cast<const CWPSTRUCT*>(lParam);
    if (msg->message == WM_SETCURSOR) {
      static uint32_t last_frame = 0;
      uint32_t frame = Unsorted::CurrentFrame;
      if (frame - last_frame > 3) {
        last_frame = frame;
        CheckCommandButtons(frame);
      }
    }
  }

  return CallNextHookEx(g_wnd_proc_hook, code, wParam, lParam);
}

static void UninstallWndProcHook() {
  if (g_wnd_proc_hook != nullptr) {
    UnhookWindowsHookEx(g_wnd_proc_hook);
    g_wnd_proc_hook = nullptr;
  }
}

static void InstallWndProcHook() {
  if (g_wnd_proc_hook != nullptr) {
    return;
  }

  DWORD thread_id = GetWindowThreadProcessId(Game::hWnd, nullptr);
  if (thread_id == 0) {
    thread_id = GetCurrentThreadId();
  }

  g_wnd_proc_hook =
      SetWindowsHookExA(WH_CALLWNDPROC, CommandButtonWndProcHook, nullptr,
                        thread_id);
  if (g_wnd_proc_hook == nullptr) {
    Log::WriteLine("Failed to install command button WndProc hook: %lu",
                   GetLastError());
    return;
  }

  std::atexit(UninstallWndProcHook);
}

DEFINE_HOOK(0x006D10AC, LoadShapeButtons, 0x6) {
  const size_t kBtnStart = 11;
  const size_t kBtnCount = 1;
  auto btn_idx_list = reinterpret_cast<int32_t*>(0x00B0CB78);
  auto btn_list = reinterpret_cast<ShapeButtonClass*>(0x00B0C1C0);
  for (size_t i = kBtnStart; i < kBtnStart + kBtnCount; i++) {
    btn_idx_list[i] = i;
    auto btn = btn_list + i;
    btn->ID = 214 + i;
    btn->Drawer = FileSystem::SIDEBAR_PAL;
    btn->IsOn = false;
    btn->ToggleType = 0;
    btn->Flags = GadgetFlag::LeftPress | GadgetFlag::LeftRelease;
    // Initial offset + i * image width
    btn->SetPosition(48 + i * 52, 736);
    SHPStruct* shp = FileSystem::LoadSHPFile("Button11.SHP");
    btn->ShapeData = shp;
    btn->Width = shp->Width;
    btn->Height = shp->Height;
    g_btns.emplace_back(btn);
  }
  InstallWndProcHook();
  return 0;
}

DEFINE_HOOK(0x006D0D4F, DrawCommandBarButtons, 0x5) {
  // for (const ShapeButtonClass* btn : g_btns) {
  //   if (btn->IsPressed) {
  //     Log::WriteLine("Button pressed");
  //   }
  // }
  return 0;
}

}  // namespace loading
}  // namespace zeus
