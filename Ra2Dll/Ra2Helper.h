#pragma once

// ��ӡ������Ϣ
void printLastError(DWORD error, const char* tag = NULL);



bool OpenLog();
void OpenMap();
void OpenRadar();
void SetBoxAllMoney();
bool IsGameRunning();