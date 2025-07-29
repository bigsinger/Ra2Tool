#include <windows.h>
#include <stdio.h>
#include <IPX.h>
#include <EventClass.h>
#include <HouseClass.h>
#include <IPXManagerClass.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"

HINSTANCE g_thisModule = NULL;		// 当前模块句柄
//////////////////////////////////////////////////
// YRpp 函数重载有问题，这里再写一遍
void MakeEventClass(EventClass* eventClass, int houseIndex, EventType eventType, int id, int rtti) {
	int typeId = static_cast<int>(eventType);
	__asm {
		pushad
		push rtti
		push id
		push typeId
		push houseIndex
		mov ecx, eventClass
		mov eax, 0x004C65E0
		call eax
		popad
	}
}
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
void OpenPsiSensor() {
	Utils::Log("OpenPsiSensor!");
#if 1
	__try {
		for (int i = 0; i < HouseClass::CurrentPlayer->Buildings.Count; i++) {
			BuildingClass* building = HouseClass::CurrentPlayer->Buildings.GetItem(i);

			if (building && building->Type) {
				Utils::LogFormat("building->Type->BuildCat: %d", building->Type->BuildCat);
			}

			if (building && building->Type && building->Type->BuildCat == BuildCat::Combat) {	// 给碉堡类建筑开启
				building->Type->PsychicDetectionRadius = 0x7FFF;
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
#else
	__try {
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
			mov word ptr[eax + 0x170C], 0x7FFF	// 0x170C PsiSensorRadius
		exit1:
			popad
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
#endif
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
// .text:004AC0D8
void RepairBuilding() {
	int count = 0;
	for (int i = 0; i < HouseClass::CurrentPlayer->Buildings.Count; i++) {
		BuildingClass* building = HouseClass::CurrentPlayer->Buildings.GetItem(i);
		if (!building->IsBeingRepaired && building->Health < building->GetType()->Strength) {
			//if (building->Health / building->GetType()->Strength < 0.7f) {
			EventClass repairEvent(0, 0);
			MakeEventClass(&repairEvent, HouseClass::CurrentPlayer->ArrayIndex, EventType::Repair,
				static_cast<int>(building->UniqueID), static_cast<int>(AbstractType::Abstract));
			EventClass::AddEvent(repairEvent);

			if ((++count) >= Config::getRepairCount()) {
				break;
			}
		}
	}
}

// 开启自动修理功能
void AutoRepair() {
	Utils::Log("Auto Repairing");
	RepairBuilding();
}
/////////////////////////////////////////

void GiveMeMoney() {
	if (SessionClass::IsMultiplayer()) { return; }

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
	if (SessionClass::IsMultiplayer()) { return; }

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

// 升级选中单位的等级为三星
void LevelUpSelectings() {
	if (SessionClass::IsMultiplayer()) { return; }

	__asm {
		pushad
		mov eax, 0x00A8ECBC
		mov ebx, [eax]				// the first selecting unit address
		mov eax, 0
	_goon:
		mov ecx, 0x00A8ECC8
		cmp eax, [ecx]				// check if selecting anything
		jge _exit1
		mov ecx, [ebx + eax * 4]	// unit address
		mov edx, 0x40000000			// float 2.0
		add ecx, 0x150
		mov[ecx], edx				// unit level
		add eax, 1
		jmp _goon

	_exit1 :
		popad
	}
}

// 强制显身，效果：幻影/间谍/隐身状态会被强显
void DisableDisguise() {
	Utils::Log("反隐功能");
	if (SessionClass::IsMultiplayer()) {
		Utils::Log("反隐功能在多人模式下会平行世界，本次不会开启，推荐在单人模式下使用。");
		return;
	}

#if 1
	for (int i = 0; i < TechnoTypeClass::Array.Count; i++) {
		TechnoTypeClass::Array.GetItem(i)->CanDisguise = false;
	}
#endif // 0


#if 0
	// 本段代码可能会平行世界
	RulesClass::Instance->DefaultMirageDisguises.Clear();
	// Unit [37]-MGTK-Mirage Tank
	TechnoTypeClass* target_unit_type = ObjectTypeClass::GetTechnoType(AbstractType::Unit, 37);
	RulesClass::Instance->DefaultMirageDisguises.AddItem(reinterpret_cast<TerrainTypeClass*>(target_unit_type));

	for (int i = 0; i < UnitClass::Array.Count; i++) {
		UnitClass* unit = UnitClass::Array.GetItem(i);
		if (unit->GetTechnoType() == target_unit_type && unit->Disguise) {
			unit->Disguise = target_unit_type;
		}
	}
#endif // 0
}

void SendGlobalMessage_sub5410F0(IPXManagerClass* self, void* buf, int buflen, int a4, IPXAddressClass* address, int a6, int a7) {
	__asm {
		pushad
		push a7
		push a6
		push address
		push a4
		push buflen
		push buf
		mov ecx, self
		mov eax, 0x005410F0
		call eax
		popad
	}
}

int32_t ComputeNameCRC(wchar_t* name) {
	int32_t crc = 0u;
	__asm {
		pushad
		mov ecx, name
		mov eax, 0x005DAC00
		call eax
		mov crc, eax
		popad
	}
	return crc;
}

// From .text:0055EDD2
void SendChatMessage(const wchar_t *message, int nCbSize) {
#if 0
	// TODO: use self for now
	NodeNameType* sender = SessionClass::Instance.Chat[0];
	GlobalPacketType packet;
	static_assert(sizeof(packet) == 455);
	packet.Command = NetCommandType::NET_MESSAGE;
	wcstombs((char *)packet.Name, sender->Name, sizeof(packet.Name));
	packet.GlobalPacketData.Message.Unknown = false;

	memcpy(packet.GlobalPacketData.Message.Buf, message, nCbSize);
	packet.GlobalPacketData.Message.PlayerColor = sender->Color;
	int32_t name_crc = ComputeNameCRC(SessionClass::Instance.GameName);
	packet.GlobalPacketData.Message.NameCRC = name_crc;

	// Broadcast.
	for (uint32_t i = 0; i < IPXManagerClass::Instance.NumConnections; i++) {
		IPXConnClass* conn = IPXManagerClass::Instance.Connection[i];
		SendGlobalMessage_sub5410F0(&IPXManagerClass::Instance, &packet, 455, 1,
			&conn->Address, 0, 0);
	}
#endif // 0
}

void Chat(const wchar_t* message, int nCbSize) {
	wchar_t msg[256] = L"我很菜 别打我~";
	SendChatMessage(msg, sizeof(msg));
}

// 测试用例
void TestCases() {
#ifdef DEVDEBUG
	Chat(NULL, 0);
	InitToolWindow();
#endif

//#include <Utilities/Debug.h>
//#include <Utilities/Macro.h>
//#include "Ext/NetHack.h"
//{ // NetHack
//	Patch::Apply_CALL(0x7B3D75, NetHack::SendTo);   // UDPInterfaceClass::Message_Handler
//	Patch::Apply_CALL(0x7B3EEC, NetHack::RecvFrom); // UDPInterfaceClass::Message_Handler
//}
}

void Install(HMODULE hModule) {
	// 设置配置文件路径
	TCHAR szPath[MAX_PATH] = { 0 };
	Utils::GetStartPath(hModule, szPath, MAX_PATH);
	_tcscat_s(szPath, MAX_PATH, _T("Ra2Dll.ini"));
	Config::setConfigFilePath(szPath);
	Utils::LogFormat("Ra2Dll Install, Build: %s", __TIMESTAMP__);

	if (Config::isOpenRA2Log()) {
		OpenLog();
	}

	TestCases();

	if (IsGaming()) {
		Utils::Log("game running!");

		if (Config::isAutoOpenMap()) {
			Utils::Log("auto open map!");
			OpenMap();
		}

		if (Config::isAutoOpenRadar()) {
			Utils::Log("auto open radar!");
			OpenRadar();
		}

		if (Config::isDisableDisguise()) {
			DisableDisguise();
		}

		InitToolWindow();
	} else {
		Utils::Log("game not running!");
	}
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

 bool IsGaming() {
	 byte value = 0;
	 __try {
		 void* p = HouseClass::CurrentPlayer;
		 value = *(byte*)p;
	 } __except (EXCEPTION_EXECUTE_HANDLER) {
		 value = 0;
	 }
	 return value;
 }

#pragma pack()
