#include "windows_shit.h"
#define EAT_SHIT_FIRST  // prevent linter move windows shit down
#include "Syringe/Syringe.h"
#include "hook.h"
#include "logging.h"
#include "memory_api.h"

// #pragma warning(push, 0)
// DEFINE_HOOK(0x004F4573, CCToolTip_Draw, 5) {
//   UNREFERENCED_PARAMETER(R);
//   static std::once_flag init_once;
//   std::call_once(init_once, [&]() {
//     ra2overlay::InitLogging(ra2overlay::LogTarget::kWindowsSink);
//   });
//   ra2overlay::hook::PostDrawCCToolTip();
//   return 0;
// }
// #pragma warning(pop)

BOOL WINAPI DllMain(HINSTANCE /*hinstDLL*/, DWORD fdwReason,
                    LPVOID lpvReserved) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
      ra2overlay::InitLogging(ra2overlay::LogTarget::kWindowsSink);
      OutputDebugStringA("[RA2OVERLAY] Process attach");
      // Install hooks.
      ra2overlay::hook::MemoryAPI::Init();
      ra2overlay::hook::HookPostDrawCCToolTip();
      break;
    }
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH: {
      OutputDebugStringA("[RA2OVERLAY] Process detach");
      if (lpvReserved != nullptr) {
        break;
      }
      break;
    }
  }
  return TRUE;
}
