#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#include "windows_shit.h"
#define EAT_SHIT_FIRST  // prevent linter move windows shit down
#include "pannel.h"

namespace ra2overlay {
namespace gui {

class Window {
 public:
  struct Params {
    std::string name;
    int width;
    int height;
    bool with_border;
    HINSTANCE hInstance;
    int nShowCmd;
    WNDPROC wndproc;
    HICON hIcon;
    HCURSOR hCursor;
  };
  struct SWFramebuffer {
    std::span<const uint8_t> buf;
    int width;
    int height;
  };

  Window();
  ~Window();
  Window(Window&& other) = delete;
  Window& operator=(Window&& other) = delete;

  bool is_gui_mouse_interacting() const { return is_gui_mouse_interacting_; }
  // glm::ivec2 resolution_size() const { return size_; }

  // glm::ivec2 frame_size() const {
  //   RECT area;
  //   GetClientRect(hwnd_, &area);
  //   int width = area.right;
  //   int height = area.bottom;
  //   return glm::ivec2(width, height);
  // }

  // glm::vec2 cursor_pos() const {
  //   POINT pos;
  //   if (GetCursorPos(&pos)) {
  //     ScreenToClient(hwnd_, &pos);
  //   }
  //   return glm::vec2(pos.x, pos.y);
  // }

  // glm::ivec2 client_pos() const {
  //   POINT pos = { 0, 0 };
  //   ClientToScreen(hwnd_, &pos);
  //   return glm::ivec2(pos.x, pos.y);
  // }

  // glm::ivec2 client_size() const {
  //   RECT area;
  //   GetClientRect(hwnd_, &area);
  //   int width = area.right;
  //   int height = area.bottom;
  //   return glm::ivec2(width, height);
  // }

  bool is_running() const { return is_running_; }

  void InitPannel(int32_t w, int32_t h);
  void UpdateTools(std::span<const ToolDesc> tools);
  void Render();
  void PollEvents();
  SWFramebuffer GetFramebuffer() const;
  void ClearScreen();
  void Close();
  void SetToolMenu(bool enable);

 private:
  bool is_running_;
  std::unique_ptr<Pannel> tool_menu_gui_ctx_;
  bool enable_tool_menu_;
  // Overlay framebuffer.
  std::vector<uint8_t> overlay_buf_;
  int overlay_buf_width_;
  int overlay_buf_height_;
  // For all tools.
  bool is_gui_mouse_interacting_;
};

}  // namespace gui
}  // namespace ra2overlay
