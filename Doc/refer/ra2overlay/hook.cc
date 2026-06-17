#include "hook.h"

#include "hook_point.h"
#include "logging.h"
#include "memory_api.h"
#include "mixer.h"
#include "window.h"

namespace ra2overlay {
namespace hook {

static std::unique_ptr<gui::Window> window;

void PostDrawCCToolTip() {
  static std::once_flag init_once;
  std::call_once(init_once, [&]() {
    window = std::make_unique<gui::Window>();
    // TODO: set pannel framebuffer size
    window->InitPannel(800, 600);
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
  });
  window->PollEvents();
  window->Render();
  gui::Window::SWFramebuffer buf = window->GetFramebuffer();
  DrawOverlay(buf.buf, buf.width, buf.height);
}

static void __declspec(naked) __cdecl InjectPostDrawCCToolTip() {
  static const uint32_t jmp_back = GetJumpBack(kHpPostDrawCCToolTip);
  __asm {
    pushad
  }
  PostDrawCCToolTip();
  __asm {
    popad
    // Original code.
    mov eax, 0x00A8B8B4
    mov eax, [eax]
    jmp [jmp_back]
  }
}

void HookPostDrawCCToolTip() {
  DLOG_F(INFO, "[RA2OVERLAY-HOOK] {}", __func__);
  MemoryAPI::instance()->HookJump(kHpPostDrawCCToolTip,
                                  InjectPostDrawCCToolTip);
}

}  // namespace hook
}  // namespace ra2overlay
