#include <windows.h>
#include <stdint.h>
#include <vector>
#include <FileFormats/SHP.h>
#include <FileSystem.h>
#include <ShapeButtonClass.h>
#include <Unsorted.h>
#include "Utils.h"
#include "Ra2Header.h"

namespace {

struct CommandButtonEntry {
	ShapeButtonClass* Button;
	int PresetIndex;
	bool WasPressed;
};

constexpr size_t COMMAND_BUTTON_START = 11;
constexpr size_t COMMAND_BUTTON_COUNT = 3;
constexpr int COMMAND_BUTTON_BASE_ID = 214;

std::vector<CommandButtonEntry> g_commandButtons;
HHOOK g_wndProcHook = NULL;
bool g_initialized = false;
bool g_createAttempted = false;
int g_lastCreateFrame = 0;

void CreateCommandButtons();

void CheckCommandButtons() {
	for (auto& entry : g_commandButtons) {
		if (!entry.Button) {
			continue;
		}

		const bool pressed = entry.Button->IsPressed;
		if (pressed && !entry.WasPressed) {
			Utils::LogFormat("CommandBar preset button %d clicked.", entry.PresetIndex);
		}
		entry.WasPressed = pressed;
	}
}

LRESULT CALLBACK CommandButtonWndProcHook(int code, WPARAM wParam, LPARAM lParam) {
	if (code >= 0) {
		const auto* msg = reinterpret_cast<const CWPSTRUCT*>(lParam);
		if (msg && msg->message == WM_SETCURSOR) {
			static int lastFrame = 0;
			const int frame = Unsorted::CurrentFrame;
			if (g_commandButtons.empty() && (!g_createAttempted || frame - g_lastCreateFrame > 300)) {
				g_createAttempted = true;
				g_lastCreateFrame = frame;
				CreateCommandButtons();
			}
			if (frame - lastFrame > 3) {
				lastFrame = frame;
				CheckCommandButtons();
			}
		}
	}

	return CallNextHookEx(g_wndProcHook, code, wParam, lParam);
}

bool InstallWndProcHook() {
	if (g_wndProcHook) {
		return true;
	}

	HWND hwnd = Game::hWnd;
	if (!hwnd) {
		hwnd = GetMainWindowForProcessId(GetCurrentProcessId());
	}

	DWORD threadId = hwnd ? GetWindowThreadProcessId(hwnd, nullptr) : 0;
	if (threadId == 0) {
		threadId = GetCurrentThreadId();
	}

	g_wndProcHook = SetWindowsHookExA(WH_CALLWNDPROC, CommandButtonWndProcHook, nullptr, threadId);
	if (!g_wndProcHook) {
		Utils::Log(GetLastError(), "CommandBarButtons SetWindowsHookEx failed.");
		return false;
	}

	return true;
}

void CreateCommandButtons() {
	__try {
		auto* buttonIndexList = reinterpret_cast<int32_t*>(0x00B0CB78);
		auto* buttonList = reinterpret_cast<ShapeButtonClass*>(0x00B0C1C0);
		SHPStruct* shp = FileSystem::LoadSHPFile("Button11.SHP");
		if (!shp) {
			Utils::Log("CommandBarButtons Button11.SHP load failed.");
			return;
		}

		g_commandButtons.clear();
		for (size_t i = 0; i < COMMAND_BUTTON_COUNT; ++i) {
			const size_t buttonIndex = COMMAND_BUTTON_START + i;
			buttonIndexList[buttonIndex] = static_cast<int32_t>(buttonIndex);

			ShapeButtonClass* button = buttonList + buttonIndex;
			button->ID = COMMAND_BUTTON_BASE_ID + static_cast<int>(buttonIndex);
			button->Drawer = FileSystem::SIDEBAR_PAL;
			button->IsOn = false;
			button->ToggleType = 0;
			button->Flags = GadgetFlag::LeftPress | GadgetFlag::LeftRelease;
			button->SetPosition(48 + static_cast<int>(buttonIndex) * 52, 736);
			button->ShapeData = shp;
			button->Width = shp->Width;
			button->Height = shp->Height;
			button->IsShapeLoaded = true;
			ShapeButtonClass::SetToolTip(button, "Ra2Tool preset");
			g_commandButtons.push_back({ button, static_cast<int>(i + 1), false });
		}

		Utils::Log("CommandBarButtons created 3 preset buttons.");
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		g_commandButtons.clear();
		Utils::Log("CommandBarButtons creation failed.");
	}
}

}

void InitCommandBarButtons() {
	if (g_initialized) {
		return;
	}

	if (InstallWndProcHook()) {
		g_initialized = true;
		Utils::Log("InitCommandBarButtons!");
	}
}

void UninitCommandBarButtons() {
	if (g_wndProcHook) {
		UnhookWindowsHookEx(g_wndProcHook);
		g_wndProcHook = NULL;
	}

	g_commandButtons.clear();
	g_initialized = false;
	g_createAttempted = false;
	g_lastCreateFrame = 0;
	Utils::Log("UninitCommandBarButtons!");
}
