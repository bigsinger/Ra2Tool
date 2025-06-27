#pragma once

// ¥Ú”°¥ÌŒÛ–≈œ¢
void printLastError(DWORD error, const char* tag = NULL);

extern HINSTANCE g_thisModule;


bool IsGameRunning();
bool OpenLog();
void OpenMap();
void OpenRadar();
void OpenTech();
void OpenPsychicDetection();
void ClearBeacons();
void GiveMeMoney();
void SetBoxAllMoney();
void DisableDisguise();
void Chat(const wchar_t* message, int nCbSize);

void Install(HMODULE hModule);
void Uninstall();