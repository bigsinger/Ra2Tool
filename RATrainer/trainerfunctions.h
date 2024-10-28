
#pragma once
#include "trainerbase.h"


class TrainerFunctions : public TrainerBase
{
public:
	//读取寄存器
	BOOL rSpeed(DWORD* pSpeed); //读取速度
	BOOL rMymoney(DWORD* pMoney);
	BOOL rBuildMaxm(DWORD* pMaxm);
	//写入寄存器
	BOOL wSpeed(DWORD Speed); //写入速度
	BOOL wMymoney(DWORD money);
	BOOL wBuildMaxm(DWORD maxm);

	BOOL QuickBuild();
	BOOL RadarOn();
	BOOL SuperOn();
	void WinImme();
	BOOL TobeGhost();

	static void Map_Assemble();
	static void RadarOn_Assemble();
	static void SetBoxAllMoney();
	static bool isGameRunning();
	static bool OpenLog();



	void AllMap();
	static void NuclearBomb_Assemble();
	void GetaNuclearBomb();
	static void UpChose_Assemble();
	void UpChose();
	static void SpeedUpChose_Assemble();
	void SpeedUpChose();
	static void MineChose_Assemble();
	void MineChose();
	static void DeleteThis_Assemble();
	void DeleteThis();
	

};
