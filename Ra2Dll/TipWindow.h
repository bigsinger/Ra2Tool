#pragma once

const int CRATE_LABEL_WIDTH = 60;  // 箱子标签宽度
const int CRATE_LABEL_HEIGHT = 40; // 箱子标签高度

HWND createCrateLabel(HWND hWndParent, LPCWSTR lpWindowName, int posX, int posY, int width = CRATE_LABEL_WIDTH, int height = CRATE_LABEL_HEIGHT);
void InitTipWindow();
void UnInitTipWindow();