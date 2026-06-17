#pragma once

#include <cstdint>
#include <span>

namespace ra2overlay {
namespace hook {

std::pair<uint32_t, uint32_t> GetSurfaceSize();
void DrawOverlay(std::span<const uint8_t> data, uint32_t width,
                 uint32_t height);

}  // namespace hook
}  // namespace ra2overlay
