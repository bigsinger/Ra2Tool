#include "raylibpp.h"

#include "logging.h"
#include "raylib.h"
#ifndef GRAPHICS_API_OPENGL_11_SOFTWARE
#error "Invalid raylib config"
#endif
#ifndef PLATFORM_DESKTOP_WIN32
#error "Invalid raylib config"
#endif
#include "rlgl.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

namespace ra2overlay {
namespace gui {

namespace {
struct GuiState {
  int render_width = 0;
  int render_height = 0;
  Font font;
};

static GuiState gs;
}  // namespace

void InitWindow(int width, int height, const char *title, void* user) {
  gs.render_width = width;
  gs.render_height = height;
  ::InitWindow(width, height, title, user);
  // TODO
  gs.font = ::LoadFont("C:\\Windows\\Fonts\\simhei.ttf");
  ::GuiSetFont(gs.font);
}

void DestructWindow() {
  ::UnloadFont(gs.font);
}

bool WindowShouldClose() {
  return ::WindowShouldClose();
}

void BeginDrawing() {
  ::BeginDrawing();
}

void EndDrawing() {
  ::EndDrawing();
}

void PollInputEvents() {
  ::PollInputEvents();
}

void SetWindowState(unsigned int flags) {
  ::SetWindowState(flags);
}

void ClearScreen(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  rlClearColor(r, g, b, a);
  rlClearScreenBuffers();
}

void CopyFramebuffer(/*ref*/ std::span<uint8_t> pixels) {
  DCHECK_EQ(pixels.size(),
            static_cast<size_t>(gs.render_width * gs.render_height * 4));
  rlCopyFramebuffer(0, 0, gs.render_width, gs.render_height,
                    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, pixels.data());
}

void ImageDrawCircle(OverlayBuf& buf, int x, int y, int radius) {
  Image img{
      .data = buf.data.data(),
      .width = buf.width,
      .height = buf.height,
      .mipmaps = 1,
      .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
  };
  ::ImageDrawCircle(&img, x, y, radius, DARKBLUE);
}

}  // namespace gui
}  // namespace ra2overlay
