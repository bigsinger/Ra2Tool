#include "base/macro.h"
__ZEUS_BEGIN_THIRD_PARTY_HEADERS
#include "FileSystem.h"
#include "ShapeButtonClass.h"
#include "Syringe.h"
__ZEUS_END_THIRD_PARTY_HEADERS
#include "Utilities/Log.h"

#include <vector>

namespace zeus {
namespace loading {

static std::vector<ShapeButtonClass*> g_btns;

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
  return 0;
}

DEFINE_HOOK(0x006D0D4F, DrawCommandBarButtons, 0x5) {
  for (const ShapeButtonClass* btn : g_btns) {
    if (btn->IsPressed) {
      Log::WriteLine("Button pressed");
    }
  }
  return 0;
}

}  // namespace loading
}  // namespace zeus
