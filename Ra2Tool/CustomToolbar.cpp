#include <windows.h>

#include "Utils.h"
#include "Config.h"
#include "OverlayToolbar/OverlayToolbar.h"

void InitCustomToolbar() {
	if (!Config::isCustomToolbarEnabled()) {
		return;
	}

	InitOverlayToolbar();
}

void UnInitCustomToolbar() {
	UninitOverlayToolbar();
}
