#pragma once


bool IsGameRunning();
bool IsGaming();
bool OpenLog();
void OpenMap();
void OpenRadar();
void OpenTech();
void OpenPsiSensor();
void ClearBeacons();
void GiveMeMoney();
void LevelUpSelectings();
void SetBoxAllMoney();
void DisableDisguise();
void Chat(const wchar_t* message, int nCbSize);

void Install(HMODULE hModule);
void Uninstall();

void InitToolWindow();
void UnInitToolWindow();

void InitTipWindow();
void UnInitTipWindow();

void AutoRepair();

void ShowCrateInfo(HDC hdc);