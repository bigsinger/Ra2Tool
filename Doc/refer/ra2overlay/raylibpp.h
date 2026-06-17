// Wrap raylib with namespace to avoid naming conflictions.
#pragma once
#include <cstdint>
#include <span>

namespace ra2overlay {
namespace gui {

void InitWindow(int width, int height, const char* title, void* user);
void DestructWindow();
bool WindowShouldClose();
void BeginDrawing();
void EndDrawing();
void PollInputEvents();
// Raylib set window state.
void SetWindowState(unsigned int flags);

void ClearScreen(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void CopyFramebuffer(/*ref*/ std::span<uint8_t> pixels);

struct OverlayBuf {
  std::span<uint8_t> data;
  int width;
  int height;
};
void ImageDrawCircle(OverlayBuf& buf, int x, int y, int radius);

}  // namespace gui
}  // namespace ra2overlay
