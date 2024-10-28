
#pragma once
#include "trainerbase.h"


class TrainerFunctions : public TrainerBase
{
public:
	//��ȡ�Ĵ���
	BOOL rSpeed(DWORD* pSpeed); //��ȡ�ٶ�
	BOOL rMymoney(DWORD* pMoney);
	BOOL rBuildMaxm(DWORD* pMaxm);
	//д��Ĵ���
	BOOL wSpeed(DWORD Speed); //д���ٶ�
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
