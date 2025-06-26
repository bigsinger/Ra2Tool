#pragma once

// ¥Ú”°¥ÌŒÛ–≈œ¢
void printLastError(DWORD error, const char* tag = NULL);

extern HINSTANCE g_thisModule;


bool OpenLog();
void OpenMap();
void OpenRadar();
void OpenTech();
void OpenPsychicDetection();
void ClearBeacons();
void GiveMeMoney();
void SetBoxAllMoney();
bool IsGameRunning();

void Install(HMODULE hModule);
void Uninstall();