#include <windows.h>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <Unsorted.h>
#include <FileSystem.h>
#include <SessionClass.h>
#include <FileFormats/SHP.h>
#include <ShapeButtonClass.h>
#include "Utils.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"

namespace {

struct CommandButtonEntry {
	ShapeButtonClass* Button;
	int PresetIndex;
	bool WasPressed;
};

constexpr size_t COMMAND_BUTTON_VISIBLE_SLOT_START = 9;
constexpr size_t COMMAND_BUTTON_OBJECT_START = 11;
constexpr int COMMAND_BUTTON_BASE_ID = 214;
constexpr int COMMAND_BUTTON_BASE_X = 48;
constexpr int COMMAND_BUTTON_STEP = 52;

using CommandButtonCallback = void (*)();

void OnCommandButtonPreset1();
void OnCommandButtonPreset2();
void OnCommandButtonPreset3();

static const char* SHPFileNames[] = {
	"Button09.SHP",
	"Button10.SHP",
	"Button11.SHP"
};

static const char* CommandButtonTooltips[] = {
	"Button Tip 1",
	"Button Tip 2",
	"Button Tip 3"
};

static CommandButtonCallback CommandButtonCallbacks[] = {
	OnCommandButtonPreset1,
	OnCommandButtonPreset2,
	OnCommandButtonPreset3
};

std::vector<CommandButtonEntry> g_commandButtons;
HHOOK g_wndProcHook = NULL;
bool g_initialized = false;
bool g_createAttempted = false;
int g_lastCreateFrame = 0;

void CreateCommandButtons();

void OnCommandButtonPreset1() {
	Utils::Log("CommandBar preset button 1 callback: PickCrate");
	StartOptimalCratePickup();
}

void OnCommandButtonPreset2() {
	Utils::Log("CommandBar preset button 2 callback.");
}

void OnCommandButtonPreset3() {
	Utils::Log("CommandBar preset button 3 callback.");
}

size_t GetCommandButtonCount() {
	return std::min(
		std::min(_countof(SHPFileNames), _countof(CommandButtonTooltips)),
		_countof(CommandButtonCallbacks));
}

bool GetGameClientSize(int* width, int* height) {
	if (!width || !height) {
		return false;
	}

	*width = 0;
	*height = 0;
	HWND hwnd = Game::hWnd;
	if (!hwnd || !IsWindow(hwnd)) {
		hwnd = GetMainWindowForProcessId(GetCurrentProcessId());
	}

	RECT client = {};
	if (!hwnd || !GetClientRect(hwnd, &client)) {
		return false;
	}

	*width = client.right - client.left;
	*height = client.bottom - client.top;
	return *width > 0 && *height > 0;
}

POINT CalculateCommandButtonStart(ShapeButtonClass* buttonList, SHPStruct* shp) {
	int clientWidth = 0;
	int clientHeight = 0;
	if (!GetGameClientSize(&clientWidth, &clientHeight)) {
		clientWidth = 1024;
		clientHeight = 768;
	}

	const int buttonWidth = shp && shp->Width > 0 ? shp->Width : 48;
	const int buttonHeight = shp && shp->Height > 0 ? shp->Height : 32;
	const int step = std::max(COMMAND_BUTTON_STEP, buttonWidth + 4);
	int x = COMMAND_BUTTON_BASE_X + static_cast<int>(COMMAND_BUTTON_VISIBLE_SLOT_START) * step;
	int y = std::max(0, clientHeight - buttonHeight);

	int detectedRight = -1;
	int detectedY = y;
	if (buttonList) {
		for (size_t i = 0; i < COMMAND_BUTTON_VISIBLE_SLOT_START; ++i) {
			ShapeButtonClass* button = buttonList + i;
			if (button->Width <= 0 || button->Height <= 0) {
				continue;
			}
			if (button->X < 0 || button->X >= clientWidth) {
				continue;
			}
			if (button->Y < clientHeight - 96 || button->Y >= clientHeight) {
				continue;
			}

			detectedRight = std::max(detectedRight, button->X + button->Width);
			detectedY = button->Y;
		}
	}

	if (detectedRight >= 0) {
		x = detectedRight + 4;
		y = detectedY;
	}

	const size_t buttonCount = GetCommandButtonCount();
	const int requiredWidth = buttonCount > 0
		? static_cast<int>(buttonCount - 1) * step + buttonWidth
		: buttonWidth;
	if (x + requiredWidth > clientWidth - 4) {
		x = std::max(4, clientWidth - requiredWidth - 4);
	}
	if (y + buttonHeight > clientHeight) {
		y = std::max(0, clientHeight - buttonHeight);
	}

	return POINT { x, y };
}

void DisableButtonFlash(ShapeButtonClass* button) {
	if (!button) {
		return;
	}

	__try {
		button->IsToFlash = false;
		button->UseFlash = false;
		button->FlashDelay = 0;
		button->FlashCounter = 0;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
}

void PrepareCommandButton(ShapeButtonClass* button, SHPStruct* shp, int x, int y, int id) {
	if (!button || !shp) {
		return;
	}

	button->ID = id;
	button->Drawer = FileSystem::SIDEBAR_PAL;
	button->IsPressed = false;
	button->IsOn = false;
	button->ToggleType = 0;
	button->Flags = GadgetFlag::LeftPress | GadgetFlag::LeftRelease;
	button->Disabled = false;
	button->NeedsRedraw = true;
	button->SetPosition(x, y);
	button->ShapeData = shp;
	button->Width = shp->Width;
	button->Height = shp->Height;
	button->IsShapeLoaded = true;
	DisableButtonFlash(button);
}

bool AreCommandButtonsValid() {
	__try {
		auto* buttonIndexList = reinterpret_cast<int32_t*>(RA2_ADDR_COMMAND_BUTTON_INDEX);
		const size_t buttonCount = GetCommandButtonCount();
		for (size_t i = 0; i < buttonCount; ++i) {
			const size_t visibleSlot = COMMAND_BUTTON_VISIBLE_SLOT_START + i;
			const size_t buttonIndex = COMMAND_BUTTON_OBJECT_START + i;
			if (buttonIndexList[visibleSlot] != static_cast<int32_t>(buttonIndex)) {
				return false;
			}
		}

		for (const auto& entry : g_commandButtons) {
			if (!entry.Button
				|| entry.Button->Disabled
				|| entry.Button->Width <= 0
				|| entry.Button->Height <= 0
				|| !entry.Button->ShapeData) {
				return false;
			}
		}
		return g_commandButtons.size() == buttonCount;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

void CheckCommandButtons() {
	for (auto& entry : g_commandButtons) {
		if (!entry.Button) {
			continue;
		}

		const bool pressed = entry.Button->IsPressed;
		DisableButtonFlash(entry.Button);
		if (pressed && !entry.WasPressed) {
			Utils::LogFormat("CommandBar preset button %d clicked.", entry.PresetIndex + 1);
			const size_t index = static_cast<size_t>(entry.PresetIndex);
			if (index < _countof(CommandButtonCallbacks) && CommandButtonCallbacks[index]) {
				CommandButtonCallbacks[index]();
			}
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
			if ((!AreCommandButtonsValid()) && (!g_createAttempted || frame - g_lastCreateFrame > 30)) {
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
		const size_t buttonCount = GetCommandButtonCount();
		if (buttonCount == 0) {
			Utils::Log("CommandBarButtons no configured buttons.");
			return;
		}

		auto* buttonIndexList = reinterpret_cast<int32_t*>(RA2_ADDR_COMMAND_BUTTON_INDEX);
		auto* buttonList = reinterpret_cast<ShapeButtonClass*>(RA2_ADDR_COMMAND_BUTTON_LIST);
		std::vector<SHPStruct*> shpFiles;
		shpFiles.reserve(buttonCount);
		int maxButtonWidth = 48;
		for (size_t i = 0; i < buttonCount; ++i) {
			SHPStruct* shp = FileSystem::LoadSHPFile(SHPFileNames[i]);
			if (!shp) {
				Utils::LogFormat("CommandBarButtons %s load failed.", SHPFileNames[i]);
				return;
			}

			shpFiles.push_back(shp);
			maxButtonWidth = std::max(maxButtonWidth, static_cast<int>(shp->Width));
		}

		const POINT start = CalculateCommandButtonStart(buttonList, shpFiles[0]);
		const int step = std::max(COMMAND_BUTTON_STEP, maxButtonWidth + 4);
		g_commandButtons.clear();
		for (size_t i = 0; i < buttonCount; ++i) {
			const size_t visibleSlot = COMMAND_BUTTON_VISIBLE_SLOT_START + i;
			const size_t buttonIndex = COMMAND_BUTTON_OBJECT_START + i;
			buttonIndexList[visibleSlot] = static_cast<int32_t>(buttonIndex);

			ShapeButtonClass* button = buttonList + buttonIndex;
			PrepareCommandButton(
				button,
				shpFiles[i],
				start.x + static_cast<int>(i) * step,
				start.y,
				COMMAND_BUTTON_BASE_ID + static_cast<int>(buttonIndex));

			ShapeButtonClass::SetToolTip(button, CommandButtonTooltips[i]);
			g_commandButtons.push_back({ button, static_cast<int>(i), false });
		}

		Utils::LogFormat(
			"CommandBarButtons created %d preset buttons: start=(%d,%d) step=%d.",
			static_cast<int>(buttonCount),
			start.x,
			start.y,
			step);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		g_commandButtons.clear();
		Utils::Log("CommandBarButtons creation failed.");
	}
}

}

void InitCommandBarButtons() {
	if (SessionClass::IsMultiplayer()) {
		Utils::Log("预制按钮在多人模式下会平行世界，本次不会开启，推荐在单人模式下使用。");
		return;
	}

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
