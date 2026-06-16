#pragma once

bool IsGameRunning();
bool IsGaming();

bool OpenLog();
void OpenMap();
void OpenRadar();
void OpenTech();
bool OpenPsiSensor(bool bforce);
void PlaceOre();
void ClearBeacons();
void GiveMeMoney();
void LevelUpSelectings();
void SetBoxAllMoney();
void DisableDisguise();
void Chat(const wchar_t* message, int nCbSize);
bool SendQuickGlobalMessage(const wchar_t* message);

void Install(HMODULE hModule);
void Uninstall();

void InitToolWindow();
void UnInitToolWindow();

void InitTipWindow();
void UnInitTipWindow();

void InitCustomToolbar();
void UnInitCustomToolbar();

void AutoRepair();

void ShowCrateInfo(HDC hdc);
void ShowEnemyPlayerInfo(HDC hdc);

void InitGrandCannonAssist();
void TickGrandCannonAssist();
void ForceGrandCannonScan();

void StartOptimalCratePickup();
void ToggleRouteCratePickup();
void TickCrateAssist();
bool IsRouteCrateCaptureActive();
bool IsRouteCrateRunActive();
void ApplySelectedFormationSquare();
void ApplySelectedFormationVertical();
void ApplySelectedFormationHorizontal();
void PrintGameMessage(const wchar_t* message);
