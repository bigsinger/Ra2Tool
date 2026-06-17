#include "window.h"

#include <cstdint>

#include "logging.h"
#include "raylibpp.h"

namespace ra2overlay {
namespace gui {

// TODO: remove useless items
Window::Window()
    : is_running_(true),
      enable_tool_menu_(true),
      overlay_buf_width_(0),
      overlay_buf_height_(0),
      is_gui_mouse_interacting_(false) {}

Window::~Window() {
  tool_menu_gui_ctx_.reset();
}

void Window::SetToolMenu(bool enable) {
  enable_tool_menu_ = enable;
  if (enable_tool_menu_) {
    LOG_F(INFO, "Enable assist tool pannel");
  } else {
    LOG_F(INFO, "Disable assist tool pannel");
  }
}

void Window::InitPannel(int32_t w, int32_t h) {
  w = std::max(0, w);
  h = std::max(0, h);
  HWND hwnd = GetForegroundWindow();
  InitWindow(w, h, "Overlay", reinterpret_cast<void*>(hwnd));
  overlay_buf_.resize(w * h * 4);
  overlay_buf_width_ = w;
  overlay_buf_height_ = h;
  tool_menu_gui_ctx_ = std::make_unique<gui::Pannel>();
}

void Window::UpdateTools(std::span<const ToolDesc> tools) {
  if (tool_menu_gui_ctx_ != nullptr) {
    tool_menu_gui_ctx_->UpdateTools(tools);
  } else {
    LOG(WARNING) << "Failed to update tools, tool menu not initialized";
  }
}

void Window::Render() {
  if (tool_menu_gui_ctx_ != nullptr) {
    is_gui_mouse_interacting_ =
        enable_tool_menu_ && tool_menu_gui_ctx_->IsMouseInteracting();
  } else {
    is_gui_mouse_interacting_ = false;
  }
  if (enable_tool_menu_) {
    BeginDrawing();
    if (tool_menu_gui_ctx_ != nullptr) {
      tool_menu_gui_ctx_->Render();
    }
    EndDrawing();
    // Get rlsw buffer.
    CopyFramebuffer(overlay_buf_);
  }
}

void Window::PollEvents() {
  if (!enable_tool_menu_) {
    return;
  }
  PollInputEvents();
  if (tool_menu_gui_ctx_ != nullptr) {
    tool_menu_gui_ctx_->HandleEvent();
  }
  is_running_ = !WindowShouldClose();
}

Window::SWFramebuffer Window::GetFramebuffer() const {
  return SWFramebuffer{
      .buf = overlay_buf_,
      .width = overlay_buf_width_,
      .height = overlay_buf_height_,
  };
}

void Window::ClearScreen() {
  gui::ClearScreen(255, 255, 255, 255);
}

void Window::Close() {
  is_running_ = false;
}

}  // namespace gui
}  // namespace ra2overlay
