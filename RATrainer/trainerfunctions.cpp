#include "windows.h"
#include "trainerfunctions.h"
#include <stdio.h>

//////////////////////////////////////////////寄存器
BOOL TrainerFunctions::rSpeed(DWORD* pSpeed) //读取速度
{
	const DWORD address = 0x00A8EB60;
	if(!readMemory(address,pSpeed))
		return FALSE;
	return TRUE;
}

BOOL TrainerFunctions::wSpeed(DWORD Speed) //写入速度
{
	const DWORD address = 0x00A8EB60;
	DWORD SpeedTest;
	if(!readMemory(address,&SpeedTest))
		return FALSE;
	writeMemory(address,Speed);
	return TRUE;
}

BOOL TrainerFunctions::rMymoney(DWORD* pMoney) //读取金钱
{
	const DWORD address[2] = {0x00A83D4C,0x30C};
	DWORD money;
	if(!readMemory(address,2,&money))
		return FALSE;
	*pMoney = money;
	return TRUE;
}

BOOL TrainerFunctions::wMymoney(DWORD money) //金钱修改
{
	const DWORD address[2] = {0x00A83D4C,0x30C};
	DWORD add;
	if(!readAddress(address,2,&add))
		return FALSE;
	writeMemory(add,money); //实际金钱地址
	return TRUE;
}


BOOL TrainerFunctions::rBuildMaxm(DWORD* pMaxm) //读取建造队列上限
{
	const DWORD address[2] = {0x008871E0,0xF0};
	DWORD maxm;
	if(!readMemory(address,2,&maxm))
		return FALSE;
	*pMaxm = maxm+1;
	return TRUE;
}

BOOL TrainerFunctions::wBuildMaxm(DWORD maxm) //上限修改
{
	const DWORD address[2] = {0x008871E0,0xF0};
	DWORD add;
	if(!readAddress(address,2,&add))
		return FALSE;
	writeMemory(add,maxm-1); //
	return TRUE;
}

BOOL TrainerFunctions::QuickBuild() //快速建造
{
	int j=0;
	DWORD dat;
	if(!readMemory(0x00A83D4C,&dat))
		return FALSE;
	for(int i=1;i<=5;(j+=4),i++)
	{
		writeMemory(dat+j+0x5378,15);
	}
	return TRUE;
}

BOOL TrainerFunctions::RadarOn() //总是有雷达
{
	const DWORD address[2] = {0x00A8B230,0x34A4};
	DWORD add;
	if(!readAddress(address,2,&add))
		return FALSE;
	writeMemory(add,0x1,0x1);
	return TRUE;
}

BOOL TrainerFunctions::SuperOn() //无限超武
{
	//00 NuclearMissile
	//01 IronCurtain
	//02 ForceShield
	//03 LightningStorm
	//04 PsychicDominator
	//05 Chronosphere
	//06 Reinforcements
	//07 SpyPlane
	//08 GeneticMutator
	//09 PsychicReveal
	//好像一共11个……
	const DWORD add_pre[2] = {0x00A83D4C,0x258};
	DWORD t1,t2,dis;	//dis->0~54

	if(!readMemory(add_pre,2,&t1)){return FALSE;}
	for(DWORD i=0; i<12; i++)		//11个循环
	{
		if(!readMemory(t1+i*4,&t2)){return FALSE;}
		if(!readMemory(t2+0x78,&dis)){return FALSE;}	//读取面板显示，没有是-1
		if((long)dis != -1)
			writeMemory(t2+0x6F,0x1,0x1);		//修改就绪标识
	}
	return TRUE;
}

void TrainerFunctions::WinImme() //立即胜利
{
	writeMemory(0x00A83D49,0x1,0x1);
}

BOOL TrainerFunctions::TobeGhost() //成为幽灵玩家
{
	DWORD dat1,dat2;
	readMemory(0x00A8ECC8,&dat1);	//选中数量
	if(dat1 != 1)		//必须选择一个建筑
		return FALSE;

	DWORD address[3] = {0x00A8ECBC,0,0};
	readMemory(address,3,&dat1);		//选中单位首地址
	if(dat1 != 0x007E3EBC)		//必须选择一个建筑
		return FALSE;

	address[2] = 0x21C;		//单位所属
	readMemory(address,3,&dat1);		//选中单位所属
	readMemory(0x00A83D4C,&dat2);		//玩家数据基址
	if(dat1 != dat2)		//必须选择玩家单位
		return FALSE;
	//满足条件后开始转移
	DWORD PlayerID[2] = {0x00A83D4C,0x30};
	readMemory(PlayerID,2,&dat1);		//获取玩家当前ID
	if(dat1 != 0)		//转移到其他ID
		dat1--;
	else
		dat1++;

	DWORD AimID[2] = {0x00A8022C,4*dat1};	//目标玩家基址
	readMemory(AimID,2,&dat2);		//获取目标玩家数据基址
	readAddress(address,3,&dat1);		//选中单位所属存储地址
	writeMemory(dat1,dat2);		//转移

	//address[2] = 0x6C;		//单位血量
	//readAddress(address,3,&dat1);		//选中单位血量存储地址
	//writeMemory(dat1,0);		//销毁选中的建筑物

	//删除选中单位
	writProcess(DeleteThis_Assemble);

	return TRUE;	
}

//////////////////////////////////////////////////线程
//全地图内联代码
#pragma pack(1)
void TrainerFunctions::Map_Assemble() {
	_asm {
		pushad
		mov eax, 0x00A83D4C
		mov edx, [eax]
		mov ecx, 0x0087F7E8
		push edx
		mov eax,0x00577D90
		call eax
		popad
	}
}

// 雷达全开 [[0x00A8B230] + 0x34A4] = 1
void TrainerFunctions::RadarOn_Assemble() {
	_asm {
		pushad
		mov eax, 0x00A8B230
		mov eax, [eax]
		mov byte ptr [eax + 0x34A4], 0x1
		popad
	}
}

// 所有捡箱子效果：金钱
void TrainerFunctions::SetBoxAllMoney() {
	const LPVOID MethodTableAddr = (LPVOID)0x004833C4;
	const size_t MethodTableCount = 0x12;
	const SIZE_T MethodTableSize = MethodTableCount * sizeof(DWORD);
	const DWORD JumpToAddr = 0x00482463;	// 捡到的是金钱的跳转地址

	// 修改代码保护属性
	DWORD dwOldProtect = 0;
	if (VirtualProtect(MethodTableAddr, MethodTableSize, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {

		DWORD* p = (DWORD*)MethodTableAddr;
		for (size_t i = 0; i < MethodTableCount; i++) {
			*(p + i) = JumpToAddr;
		}

		// 恢复代码保护属性
		if (!VirtualProtect(MethodTableAddr, MethodTableSize, dwOldProtect, &dwOldProtect)) {
			printLastError(GetLastError(), "Failed 2!");
		}
	} else {
		printLastError(GetLastError(), "Failed 1!");
	}
}

#pragma optimize("", off)
BOOL installHook(LPVOID addr, DWORD newFunc) {
	byte buff[5];
	DWORD* offset = (DWORD*) & buff[1];
	DWORD dwWrittenSize = 0;
	DWORD dwOldProtect = 0;

	buff[0] = 0xE9;	// jmp
	*offset = newFunc - (DWORD)addr - 5;

	if (VirtualProtect(addr, 5u, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
		WriteProcessMemory(GetCurrentProcess(), addr, &buff, 5u, &dwWrittenSize);
		if (!VirtualProtect(addr, 5u, dwOldProtect, &dwOldProtect)) {
			printLastError(GetLastError(), "Failed 2!");
		}
	} else {
		printLastError(GetLastError(), "Failed 1!");
	}

	return true;	
}

void __cdecl log(const char* Format, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9) {
	char buff1[1024] = {};
	char buff2[1024] = {};
	sprintf_s(buff1, Format, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	sprintf_s(buff2, "[Ra2] %s", buff1);
	OutputDebugStringA(buff2);
}

byte backupCode[5] = {};
bool TrainerFunctions::OpenLog() {
	OutputDebugStringA("[Ra2] OpenLog");

	void* oldFunc = (byte*)GetModuleHandleA(0) + 0x68E0; // 0x4068E0 是一个空函数，就一个retn
	memcpy(&backupCode, oldFunc, 5u);
	return installHook(oldFunc, (DWORD)&log);
}
#pragma optimize("", on)


// 判断游戏是否运行
 bool TrainerFunctions::isGameRunning() {
	 bool isRunning = false;

	 __try {
		 _asm {
			 pushad
			 mov eax, 0x00A83D4C
			 mov eax, [eax]
			 mov eax, 0x0087F7E8
			 mov eax, [eax]
			 popad
		 }

		 isRunning = true;
	 } __except (EXCEPTION_EXECUTE_HANDLER) {
		 isRunning = false;
	 }
	 return isRunning;
}

#pragma pack()

void TrainerFunctions::AllMap() {
	//地图全开
	writProcess(Map_Assemble);
}



//额外核弹内联代码
void TrainerFunctions::NuclearBomb_Assemble()
{
	int eax_temp;       
	__asm
	{
		pushad
		mov ecx,0x00A83D4C	//检查是否已经有核弹发射井
		mov ecx,[ecx]
		add ecx,0x258
		mov ecx,[ecx]
		mov ecx,[ecx]
		add ecx,0x78
		mov ecx,[ecx]
		cmp ecx,0xFFFFFFFF	//面板-1表示没有 0~54表示有
		jne exit1	//有就退出防止出错

		mov ecx,0x14
		mov eax_temp,0x006CEEB0
		call eax_temp
		mov ebx,eax
		//调整核弹CD
		add ebx,0x98
		mov eax,[ebx]//超武编号
		mov edx,0x00A83D4C
		mov edx,[edx]
		add edx,0x258
		mov ecx,[edx]
		push 0           //初始状态，1=等待
		push 0           //对应建筑
		mov ecx,[ecx+eax*4]
		push 1             //1=一次性 核弹必须1 其他可以0
		mov eax_temp,0x006CB560
		call eax_temp
		//增加核弹攻击选项
		add ebx,0x98
		mov eax,[ebx]//超武编号
		mov ecx,0x0087F7E8
		push eax
		push 0x1F
		mov eax_temp,0x006A6300
		call eax_temp

		exit1:
		popad             
	}       
}

void TrainerFunctions::GetaNuclearBomb()
{
	//额外核弹
	writProcess(NuclearBomb_Assemble);
}

//单位升级内联汇编
void TrainerFunctions::UpChose_Assemble()
{
	__asm
	{
		pushad
		mov eax,0x00A8ECBC
		mov ebx,[eax]	//选中单位首地址
		mov eax,0

		process:
		mov ecx,0x00A8ECC8
		cmp eax,[ecx]  //选中单位数量为零判断
		jge brek//>=跳转
		mov ecx,[ebx+eax*4]  //当前单位地址
		mov edx,0x40000000		//Float 2.0
		add ecx,0x150
		mov [ecx],edx  //当前单位经验
		add eax,1
		jmp process

		brek:
		popad
	}
}

void TrainerFunctions::UpChose()
{
	//单位升级
	writProcess(UpChose_Assemble);
}

//单位加速内联汇编
void TrainerFunctions::SpeedUpChose_Assemble()
{
	__asm
	{
		pushad
		mov eax,0x00A8ECBC
		mov ebx,[eax]	//选中单位首地址
		mov eax,0

		process:
		mov ecx,0x00A8ECC8
		cmp eax,[ecx]  //选中单位数量
		jge brek//>=跳转
		mov ecx,[ebx+eax*4]  //当前单位地址
		mov edx,0x007E3EBC
		cmp [ecx],edx	//当前单位类型为建筑
		je brek			//建筑没速度不能改
		mov edx,0x007EB058
		cmp [ecx],edx	//步兵最大改2.0
		jne next1
		mov edx,0x40000000
		add ecx,0x584
		mov [ecx],edx
		je loopcontinue
		next1:
		mov edx,0x007F5C70
		cmp [ecx],edx	//车船
		jne next2
		mov edx,0x40A00000
		add ecx,0x584
		mov [ecx],edx	//5.0
		je loopcontinue
		next2:
		//飞机改这个没用
		//mov [ecx+584],3FF00000//1.875
		loopcontinue:
		add eax,1
		jmp process

		brek:
		popad
	}
}

void TrainerFunctions::SpeedUpChose()
{
	//单位加速
	writProcess(SpeedUpChose_Assemble);
}

//全是我的-选中单位 内联汇编
void TrainerFunctions::MineChose_Assemble()
{
	__asm
	{
		pushad
		mov eax,0x00A8ECC8	//选中单位数量
		mov eax,[eax]
		cmp eax,0		//是否选中单位
		je exit1
		push 0  //
		mov ebx,0x00A83D4C
		mov eax,[ebx]
		push eax
		mov ebx,0x00A8ECBC
		mov eax,[ebx]
		mov ecx,[eax]
		mov ebx,[ecx]
		add ebx,0x3D4
		mov ebx,[ebx]
		call ebx
		exit1:
		popad
	}
}

void TrainerFunctions::MineChose()
{
	//全是我的-选中单位
	writProcess(MineChose_Assemble);
}

//删除选中单位 内联汇编
void TrainerFunctions::DeleteThis_Assemble()
{
	__asm
	{
		pushad
		mov eax,0x00A8ECC8
		mov eax,[eax]//选中单位数量
		cmp eax,0		//是否选中单位
		je exit1

		mov eax,0x00A8ECBC
		mov eax,[eax]
		mov ecx,[eax]
		mov eax,[ecx]
		add eax,0xF8
		mov ebx,[eax]
		call ebx

		exit1:
		popad
	}
}

void TrainerFunctions::DeleteThis()
{
	//删除选中单位
	writProcess(DeleteThis_Assemble);
}
