#pragma once

#include <YRPP.h>
#include <Helpers/CompileTime.h>

class NOVTABLE EvadeClass
{
public:
	DEFINE_REFERENCE(EvadeClass, Instance, 0x8A38E0)

	void Do() { JMP_THIS(0x4C6210); }

	//Properties
public:
	char  CarryOverGlobals[50];
	int   CarryOverMoney;
	int   CarryOverTimer;
	int   CarryOverDifficulty;
	short CarryOverStage;
};
