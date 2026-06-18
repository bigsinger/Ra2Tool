#pragma once

class EventClass;
enum class EventType : unsigned char;

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
void Chat(const wchar_t* message);

void Install(HMODULE hModule);
void Uninstall();

void InitToolWindow();
void UnInitToolWindow();

void InitTipWindow();
void UnInitTipWindow();

void InitCustomToolbar();
void UnInitCustomToolbar();
void InitCommandBarButtons();
void UninitCommandBarButtons();

void AutoRepair();

void ShowCrateInfo(HDC hdc);
void ShowEnemyPlayerInfo(HDC hdc);
void DrawEnemyInfoOverlay();

void InitGrandCannonAssist();
void TickGrandCannonAssist();
void ForceGrandCannonScan();

void StartOptimalCratePickup();
void ToggleRouteCratePickup();
void StartRouteCratePickupInArea(short left, short top, short right, short bottom);
void TickCrateAssist();
bool IsRouteCrateCaptureActive();
bool IsRouteCrateRunActive();
bool GetRouteCrateArea(short* left, short* top, short* right, short* bottom);
void ApplySelectedFormationSquare();
void ApplySelectedFormationVertical();
void ApplySelectedFormationHorizontal();
void PrintGameMessage(const wchar_t* message);
void MakeEventClass(EventClass* eventClass, int houseIndex, EventType eventType, int id, int rtti);
