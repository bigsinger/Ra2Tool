// List all hook points here to detect conflictions easily.
// Assume the executable's image base is always 0x00400000.
#pragma once
#include <cstdint>
#include <utility>

namespace ra2overlay {
namespace hook {
// Hook at where and how many asm code bytes the hook point occupies.
using HookPoint = std::pair<uint32_t /*addr*/, uint32_t /*bytes*/>;

constexpr uint32_t GetJumpBack(HookPoint hp) {
  return hp.first + hp.second;
}

static constexpr HookPoint kHpPostDrawCCToolTip = {0x004F4573, 5};

}  // namespace hook
}  // namespace ra2overlay
