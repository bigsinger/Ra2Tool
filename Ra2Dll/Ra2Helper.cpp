#include "windows.h"
#include <stdio.h>
#include "Ra2Helper.h"
#include "ToolWindow.h"
#include <EventClass.h>
#include <HouseClass.h>
#include "Utils.h"
#include "Config.h"

HINSTANCE g_thisModule = NULL;
//////////////////////////////////////////////////


//全地图内联代码
#pragma pack(1)
void OpenMap() {
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
void OpenRadar() {
	_asm {
		pushad
		mov eax, 0x00A8B230
		mov eax, [eax]
		mov byte ptr [eax + 0x34A4], 0x1
		popad
	}
}

void _openTechOne(int objType, int index) {
	_asm {
		pushad
		mov ecx, 0x0087F7E8
		push index
		push objType
		mov eax, 0x006A6300
		call eax
		popad
	}
}

void _openTechAll(int objType, int count) {
	_asm {
		pushad
		xor esi, esi
	label:
		push esi
		push objType			// 0x7是建筑；0x10(也可能是0xF)是步兵；0x28是车船；0x1F=武器
		mov ecx, 0x0087F7E8
		mov eax, 0x006A6300
		call eax
		inc esi
		cmp esi, count			// 可以填一个编号上限值，参考上述ini
		jle label
		popad
	}
}

// 科技全开
void OpenTech() {
	//_openTechAll(7, 69);				// 建筑全开
	//_openTechAll(0x10, 30);			// 步兵全开
	//_openTechAll(0x28, 40);			// 坦克全开

	_openTechOne(0x28, 0);		// 盟军基地车
	_openTechOne(0x28, 26);	// 苏军基地车
	//_openTechOne(0x28, 54);			// 尤里基地车

	_openTechOne(0x28, 2);		// 天启
	_openTechOne(0x28, 14);	// V3
	//_openTechOne(0x28, 15);	// 基洛夫
	_openTechOne(0x28, 16);	// 恐怖机器人（蜘蛛）
	_openTechOne(0x28, 27);	// 坦克杀手
	_openTechOne(0x28, 34);	// 光棱坦克
	_openTechOne(0x28, 36);	// 幻影
	_openTechOne(0x28, 37);	// 多功能
}

// 开启心灵探测
void OpenPsychicDetection() {
	_asm {
		pushad
		mov eax, 0x00A8ECC8
		mov eax, [eax]					//选中单位的数量
		cmp eax, 0						//是否有选中单位
		je exit1
		mov eax, 0x00A8ECBC
		mov eax, [eax]
		mov eax, [eax]					// +0
		mov eax, [eax]					// +0
		cmp eax, 0x007E3EBC				// Building vt
		jnz exit1
		mov eax, 0x00A8ECBC
		mov eax, [eax]
		mov eax, [eax]					// +0
		mov eax, [eax + 0x520]			// +0x520
		mov word ptr[eax + 0x170C], 0x7FFF	// 0x170C PsychicDetectionRadius
	exit1:
		popad
	}
}

// 删除信标：每次调用只清除一个信标
void _clearBeacon() {
	_asm {
		pushad
		mov ecx, 0x0089C3B0
		push - 1
		push 0
		mov eax, 0x004311C0
		call eax
		popad
	}
}

// 清除信标：一次批量清除玩家自己的3个信标
// 004FC1C6: sub_431410(dword_89C3B0, *(_DWORD *)(dword_A83D4C + 0x30));
void ClearBeacons() {
	_asm {
		pushad
		mov     eax, 0x00A83D4C
		mov     eax, [eax]
		mov     eax, [eax + 0x30]
		push    eax
		mov     ecx, 0x0089C3B0
		mov eax, 0x00431410
		call eax
		popad
	}
}

/////////////////////////////////////////

void GiveMeMoney() {
	_asm {
		pushad
		mov eax, 0x00A83D4C
		mov eax, [eax]
		mov dword ptr[eax + 0x30C], 0x7FFFFFFF
		popad
	}
}

// 所有捡箱子效果：金钱
void SetBoxAllMoney() {
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
			Utils::Log(GetLastError(), "Failed 2!");
		}
	} else {
		Utils::Log(GetLastError(), "Failed 1!");
	}
}

void Install(HMODULE hModule) {
	Utils::Log("Ra2Dll Install");

	// 设置配置文件路径
	TCHAR szPath[MAX_PATH] = { 0 };
	Utils::GetStartPath(hModule, szPath, MAX_PATH);
	_tcscat_s(szPath, MAX_PATH, _T("Ra2Dll.ini"));
	Config::SetConfigFilePath(szPath);

	if (Config::isOpenRA2Log()) {
		OpenLog();
	}

	InitToolWindow();
}

// 卸载时调用
void Uninstall() {
	UnInitToolWindow();
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
			Utils::Log(GetLastError(), "Failed 2!");
		}
	} else {
		Utils::Log(GetLastError(), "Failed 1!");
	}

	return true;	
}

void __cdecl ra2log(const char* Format, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9) {
	if (!strstr(Format, "Theme")) {
		char buff1[1024] = {};
		char buff2[1024] = {};
		DWORD* pArgs = (DWORD*)((DWORD)&Format - 4);	//&p是参数栈的地址，&p-4是返回地址

		sprintf_s(buff1, Format, a1, a2, a3, a4, a5, a6, a7, a8, a9);
		sprintf_s(buff2, "[Ra2] %08X %s", pArgs[0], buff1);
		OutputDebugStringA(buff2);
	}
}

byte backupCode[5] = {};
bool OpenLog() {
	OutputDebugStringA("[Ra2] OpenLog");

	void* oldFunc = (byte*)GetModuleHandleA(0) + 0x68E0; // 0x4068E0 是一个空函数，就一个retn
	memcpy(&backupCode, oldFunc, 5u);
	return installHook(oldFunc, (DWORD)&ra2log);
}
#pragma optimize("", on)


// 判断游戏是否运行
 bool IsGameRunning() {
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
