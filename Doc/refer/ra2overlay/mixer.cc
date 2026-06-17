#include "mixer.h"

#include <cassert>

#include "windows_shit.h"
#define EAT_SHIT_FIRST  // prevent linter move windows shit down
#include "ddraw.h"
#include "Surface.h"

namespace ra2overlay {
namespace hook {

void ApplyOverlay(IDirectDrawSurface* surf, DDSURFACEDESC* desc,
                  std::span<const uint8_t> data, uint32_t width,
                  uint32_t height) {
  HRESULT res =
      surf->Lock(NULL, desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
  assert(res == DD_OK);
  // Get surface pointer and calculate pitch
  uint8_t* surfacePtr = static_cast<uint8_t*>(desc->lpSurface);
  uint32_t surfacePitch = desc->lPitch;
  // Process each pixel
  for (uint32_t y = 0; y < height; ++y) {
    // Get pointers to current row in both surfaces
    uint16_t* destRow =
        reinterpret_cast<uint16_t*>(surfacePtr + y * surfacePitch);
    const uint8_t* srcRow =
        data.data() + y * width * 4;  // 4 bytes per pixel (RGBA)
    for (uint32_t x = 0; x < width; ++x) {
      const uint8_t* srcPixel = srcRow + x * 4;
      uint8_t r = srcPixel[0];
      uint8_t g = srcPixel[1];
      uint8_t b = srcPixel[2];
      uint8_t a = srcPixel[3];
      // Alpha blending: if alpha is 0, keep destination; if alpha is 255, use
      // source For intermediate alpha values, blend between source and
      // destination
      if (a == 0) {
        // Fully transparent - skip this pixel (keep destination)
        continue;
      } else if (a == 255) {
        // Fully opaque - use source pixel
        // Convert RGB888 to RGB565
        uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        destRow[x] = rgb565;
      } else {
        // Alpha blending
        uint16_t destPixel = destRow[x];
        // Extract RGB565 components from destination
        uint8_t dest_r = (destPixel >> 11) & 0x1F;
        uint8_t dest_g = (destPixel >> 5) & 0x3F;
        uint8_t dest_b = destPixel & 0x1F;
        // Convert to 8-bit for blending (scale 5/6-bit to 8-bit)
        dest_r = (dest_r << 3) | (dest_r >> 2);
        dest_g = (dest_g << 2) | (dest_g >> 4);
        dest_b = (dest_b << 3) | (dest_b >> 2);
        // Alpha blending
        float alpha = a / 255.0f;
        float inv_alpha = 1.0f - alpha;
        uint8_t blended_r =
            static_cast<uint8_t>(r * alpha + dest_r * inv_alpha);
        uint8_t blended_g =
            static_cast<uint8_t>(g * alpha + dest_g * inv_alpha);
        uint8_t blended_b =
            static_cast<uint8_t>(b * alpha + dest_b * inv_alpha);
        // Convert back to RGB565
        uint16_t rgb565 = ((blended_r & 0xF8) << 8) |
                          ((blended_g & 0xFC) << 3) | (blended_b >> 3);
        destRow[x] = rgb565;
      }
    }
  }
  res = surf->Unlock(NULL);
  assert(res == DD_OK);
}

std::pair<uint32_t, uint32_t> GetSurfaceSize() {
  yrpp::DSurface* surf = yrpp::DSurface::Composite.get();
  return {surf->Width, surf->Height};
}

void DrawOverlay(std::span<const uint8_t> data, uint32_t width,
                 uint32_t height) {
  yrpp::DSurface* surf = yrpp::DSurface::Composite.get();
  ApplyOverlay(surf->VideoSurfacePtr,
               reinterpret_cast<DDSURFACEDESC*>(surf->VideoSurfaceDescription),
               data, width, height);
}

}  // namespace hook
}  // namespace ra2overlay
