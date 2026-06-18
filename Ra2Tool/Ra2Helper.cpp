#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <IPX.h>
#include <EventClass.h>
#include <HouseClass.h>
#include <IPXManagerClass.h>
#include <MessageListClass.h>
#include <RulesClass.h>
#include <SessionClass.h>
#include <ASMMacros.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"

HINSTANCE g_thisModule = NULL;		// 当前模块句柄

#pragma pack(push, 1)
struct ChatGlobalPacket {
	int Command;
	wchar_t Name[22];
	wchar_t Message[112];
	int PlayerColor;
	BYTE Reserved[8];
	int NameCRC;
	BYTE Padding[455 - 0x120];
};
#pragma pack(pop)

static_assert(sizeof(ChatGlobalPacket) == 455);
static_assert(offsetof(ChatGlobalPacket, Command) == 0x0);
static_assert(offsetof(ChatGlobalPacket, Name) == 0x4);
static_assert(offsetof(ChatGlobalPacket, Message) == 0x30);
static_assert(offsetof(ChatGlobalPacket, PlayerColor) == 0x110);
static_assert(offsetof(ChatGlobalPacket, NameCRC) == 0x11C);
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
		mov eax, RA2_FUNC_EVENT_TARGET_CTOR
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
		mov eax, RA2_ADDR_CURRENT_HOUSE
		mov edx, [eax]
		mov ecx, RA2_ADDR_MOUSE_CLASS
		push edx
		mov eax, RA2_FUNC_REVEAL_MAP
		call eax
		popad
	}
}

// 雷达全开 [[RA2_ADDR_SCENARIO_CLASS] + 0x34A4] = 1
void OpenRadar() {
	_asm {
		pushad
		mov eax, RA2_ADDR_SCENARIO_CLASS
		mov eax, [eax]
		mov byte ptr [eax + 0x34A4], 0x1
		popad
	}
}

void _openTechOne(int objType, int index) {
	_asm {
		pushad
		mov ecx, RA2_ADDR_MOUSE_CLASS
		push index
		push objType
		mov eax, RA2_FUNC_REVEAL_TECH
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
		mov ecx, RA2_ADDR_MOUSE_CLASS
		mov eax, RA2_FUNC_REVEAL_TECH
		call eax
		inc esi
		cmp esi, count			// 可以填一个编号上限值，参考上述ini
		jle label
		popad
	}
}

void addCameoSafely(AbstractType absType, int idxType) {
	__try {
		SidebarClass::Instance.AddCameo(absType, idxType);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::LogFormat("AddCameo Error! absType: %d idxType: $d", absType, idxType);
	}
}

// 科技全开
void OpenTech() {
	if (Config::isOpenTechUsingCustom()) {
		Utils::Log("Using Custom Config Tech!");

		// 开启用户配置的建筑
		auto buildings = Config::getBuildingTypeTechList();
		for (auto i : buildings) {
			addCameoSafely(AbstractType::BuildingType, i);
		}

		// 开启用户配置的步兵
		auto infantry = Config::getInfantryTypeTechList();
		for (auto i : infantry) {
			addCameoSafely(AbstractType::InfantryType, i);
		}

		// 开启用户配置的单位
		auto units = Config::getUnitTypeTechList();
		for (auto i : units) {
			addCameoSafely(AbstractType::UnitType, i);
		}
	} else {
		Utils::Log("Using Predefined Config Tech!");

		auto techno_type = TechnoTypeClass::Array.GetItem(71);
		auto building_type = abstract_cast<BuildingTypeClass*>(techno_type);
		techno_type->OwnerFlags = 0x7FFFFFFF;
		building_type->NeedsEngineer = false;
		SidebarClass::Instance.AddCameo(AbstractType::BuildingType, 64); // 巨炮
		SidebarClass::Instance.AddCameo(AbstractType::BuildingType, 71); // 科技钻油厂
		SidebarClass::Instance.AddCameo(AbstractType::BuildingType, 73);	// 矿石精鍊器

		_openTechOne(0x28, 0);		// 盟军基地车
		_openTechOne(0x28, 26);	// 苏军基地车
		//_openTechOne(0x28, 54);			// 尤里基地车
		SidebarClass::Instance.AddCameo(AbstractType::UnitType, 16); // 恐怖机器人（蜘蛛）
		SidebarClass::Instance.AddCameo(AbstractType::UnitType, 27); // 坦克杀手

		_openTechOne(0x28, 2);		// 天启
		_openTechOne(0x28, 14);	// V3

		//_openTechOne(0x28, 15);	// 基洛夫
		_openTechOne(0x28, 34);	// 光棱坦克
		_openTechOne(0x28, 36);	// 幻影
		_openTechOne(0x28, 38);	// 多功能

		SidebarClass::Instance.AddCameo(AbstractType::InfantryType, 8);	// 生化工兵
		SidebarClass::Instance.AddCameo(AbstractType::InfantryType, 16);	// spy
	}
}

// 开启心灵探测
bool OpenPsiSensor(bool bforce) {
	Utils::Log("OpenPsiSensor!");
	static bool isPsiSensorOpen = false;
	if (isPsiSensorOpen && !bforce) { return true; }

#if 1
	__try {
		for (int i = 0; i < HouseClass::CurrentPlayer->Buildings.Count; i++) {
			BuildingClass* building = HouseClass::CurrentPlayer->Buildings.GetItem(i);
			if (building && building->Type && building->Type->BuildCat == BuildCat::Combat) {	// 给碉堡类建筑开启
				building->Type->PsychicDetectionRadius = 0x7FFF;
				isPsiSensorOpen = true;
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
#else
	__try {
		_asm {
			pushad
			mov eax, RA2_ADDR_SELECTED_COUNT
			mov eax, [eax]					//选中单位的数量
			cmp eax, 0						//是否有选中单位
			je exit1
			mov eax, RA2_ADDR_SELECTED_OBJECTS
			mov eax, [eax]
			mov eax, [eax]					// +0
			mov eax, [eax]					// +0
			cmp eax, RA2_VTABLE_BUILDING				// Building vt
			jnz exit1
			mov eax, RA2_ADDR_SELECTED_OBJECTS
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

	return isPsiSensorOpen;
}

// 放置矿石精炼厂
void PlaceOre() {
	Utils::Log("PlaceOre");
	__try {
		// 会提示：cannot deploy here，因此无法随意放置，暂时不开启了。
		CellStruct cell{ 10, 10 };
		EventClass event(HouseClass::CurrentPlayer->ArrayIndex, EventType::Place, AbstractType::Building, 73, false, cell);
		EventClass::OutList.Add(event);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::Log("PlaceOre Error");
	}
}

// 删除信标：每次调用只清除一个信标
void _clearBeacon() {
	_asm {
		pushad
		mov ecx, RA2_ADDR_BEACON_MANAGER
		push - 1
		push 0
		mov eax, RA2_FUNC_CLEAR_BEACON_ONE
		call eax
		popad
	}
}

// 清除信标：一次批量清除玩家自己的3个信标
// 游戏内部也通过 beacon manager 和当前玩家索引批量删除信标。
void ClearBeacons() {
	_asm {
		pushad
		mov     eax, RA2_ADDR_CURRENT_HOUSE
		mov     eax, [eax]
		mov     eax, [eax + 0x30]
		push    eax
		mov     ecx, RA2_ADDR_BEACON_MANAGER
		mov eax, RA2_FUNC_CLEAR_BEACONS_BY_HOUSE
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
			EventClass::OutList.Add(repairEvent);

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
		mov eax, RA2_ADDR_CURRENT_HOUSE
		mov eax, [eax]
		mov dword ptr[eax + 0x30C], 0x7FFFFFFF
		popad
	}
}

// 所有捡箱子效果：金钱
void SetBoxAllMoney() {
	if (SessionClass::IsMultiplayer()) { return; }

	const LPVOID MethodTableAddr = (LPVOID)RA2_FUNC_CRATE_METHOD_TABLE;
	const size_t MethodTableCount = 0x12;
	const SIZE_T MethodTableSize = MethodTableCount * sizeof(DWORD);
	const DWORD JumpToAddr = RA2_FUNC_CRATE_MONEY_EFFECT;	// 捡到的是金钱的跳转地址

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
		mov eax, RA2_ADDR_SELECTED_OBJECTS
		mov ebx, [eax]				// the first selecting unit address
		mov eax, 0
	_goon:
		mov ecx, RA2_ADDR_SELECTED_COUNT
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

#if 0
	// 本段代码可能会平行世界
	for (int i = 0; i < TechnoTypeClass::Array.Count; i++) {
		TechnoTypeClass::Array.GetItem(i)->CanDisguise = false;
	}
#endif // 0

#if 1
	// 本段代码可能会平行世界
	RulesClass::Instance->DefaultMirageDisguises.Clear();
	// Unit [37]-MGTK-Mirage Tank
	TechnoTypeClass* target_unit_type = ObjectTypeClass::GetTechnoType(AbstractType::Unit, 37);
	RulesClass::Instance->DefaultMirageDisguises.AddItem(reinterpret_cast<TerrainTypeClass*>(target_unit_type));

	for (int i = 0; i < UnitClass::Array.Count; i++) {
		UnitClass* unit = UnitClass::Array.GetItem(i);
		if (unit->GetTechnoType() == target_unit_type && unit->Disguise) {
			unit->Disguise = target_unit_type;	// null ?
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
		mov eax, RA2_FUNC_SEND_GLOBAL_MESSAGE
		call eax
		popad
	}
}

int32_t ComputeNameCRC(wchar_t* name) {
	int32_t crc = 0u;
	__asm {
		pushad
		mov ecx, name
		mov eax, RA2_FUNC_COMPUTE_NAME_CRC
		call eax
		mov crc, eax
		popad
	}
	return crc;
}

int IPXNumConnections(IPXManagerClass* self) {
	int result = 0;
	__asm {
		mov ecx, self
		mov eax, RA2_FUNC_IPX_NUM_CONNECTIONS
		call eax
		mov result, eax
	}
	return result;
}

int IPXConnectionID(IPXManagerClass* self, int index) {
	int result = 0;
	__asm {
		push index
		mov ecx, self
		mov eax, RA2_FUNC_IPX_CONNECTION_ID
		call eax
		mov result, eax
	}
	return result;
}

wchar_t* IPXConnectionPlayerName(IPXManagerClass* self, int connectionId) {
	wchar_t* result = nullptr;
	__asm {
		push connectionId
		mov ecx, self
		mov eax, RA2_FUNC_IPX_CONNECTION_NAME
		call eax
		mov result, eax
	}
	return result;
}

IPXAddressClass* IPXConnectionAddress(IPXManagerClass* self, int connectionId) {
	IPXAddressClass* result = nullptr;
	__asm {
		push connectionId
		mov ecx, self
		mov eax, RA2_FUNC_IPX_CONNECTION_ADDRESS
		call eax
		mov result, eax
	}
	return result;
}

void IPXService(IPXManagerClass* self) {
	__asm {
		mov ecx, self
		mov eax, RA2_FUNC_IPX_SERVICE
		call eax
	}
}

NodeNameType* FindCurrentNodeName() {
	__try {
		const int houseIndex = HouseClass::CurrentPlayer ? HouseClass::CurrentPlayer->ArrayIndex : -1;
		for (int i = 0; i < NodeNameType::Array.Count; ++i) {
			NodeNameType* node = NodeNameType::Array.GetItem(i);
			if (node && node->HouseIndex == houseIndex) {
				return node;
			}
		}

		return NodeNameType::Array.Count > 0 ? NodeNameType::Array.GetItem(0) : nullptr;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

inline unsigned int __stdcall GetColorEnum_69A310(unsigned int color) {
	JMP_STD(RA2_FUNC_COLOR_ENUM);
}

void CopyCurrentChatIdentity(wchar_t* displayName, size_t displayNameCount, int* color) {
	if (!displayName || displayNameCount == 0 || !color) {
		return;
	}

	displayName[0] = L'\0';
	*color = SessionClass::Instance.PlayerColor;

	__try {
		NodeNameType* sender = FindCurrentNodeName();
		if (sender) {
			wcsncpy_s(displayName, displayNameCount, sender->Name, _TRUNCATE);
			*color = sender->Color;
			return;
		}

		MultiByteToWideChar(CP_ACP, 0, SessionClass::Instance.Handle, -1,
			displayName, static_cast<int>(displayNameCount));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}

	if (!displayName[0]) {
		wcscpy_s(displayName, displayNameCount, L"玩家");
	}
}

void DisplayChatMessage(const wchar_t* displayName, int color, const wchar_t* message) {
	if (!message || !*message) {
		return;
	}

	__try {
		const TextPrintType style =
			TextPrintType::UseGradPal |
			TextPrintType::FullShadow |
			TextPrintType::Point6Grad;
		const int delay = RulesClass::Instance
			? static_cast<int>(RulesClass::Instance->MessageDelay * 900.0)
			: 0x96;

		MessageListClass::Instance.AddMessage(
			displayName,
			color,
			message,
			static_cast<int>(GetColorEnum_69A310(color)),
			style,
			delay,
			!SessionClass::IsMultiplayer());
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		MessageListClass::Instance.PrintMessage(message);
	}
}

bool BuildChatGlobalPacket(const wchar_t* message, ChatGlobalPacket* packet) {
	if (!message || !*message || !packet) {
		return false;
	}

	memset(packet, 0, sizeof(*packet));
	packet->Command = NET_MESSAGE;
	CopyCurrentChatIdentity(packet->Name, _countof(packet->Name), &packet->PlayerColor);
	wcsncpy_s(packet->Message, message, _TRUNCATE);
	packet->NameCRC = ComputeNameCRC(packet->Name);
	return true;
}

int SendChatPacketToConnections(ChatGlobalPacket* packet) {
	if (!packet || !SessionClass::IsMultiplayer()) {
		return 0;
	}

	int sentCount = 0;
	__try {
		IPXManagerClass* ipx = &IPXManagerClass::Instance;
		const int connectionCount = IPXNumConnections(ipx);
		if (connectionCount <= 0) {
			return 0;
		}

		for (int houseIndex = 0; houseIndex < HouseClass::Array.Count; ++houseIndex) {
			HouseClass* house = HouseClass::Array.GetItem(houseIndex);
			if (!house || !house->Type || !house->Type->Multiplay || !house->UIName[0]) {
				continue;
			}

			for (int connectionIndex = 0; connectionIndex < connectionCount; ++connectionIndex) {
				const int connectionId = IPXConnectionID(ipx, connectionIndex);
				wchar_t* connectionName = IPXConnectionPlayerName(ipx, connectionId);
				if (!connectionName || wcscmp(house->UIName, connectionName) != 0) {
					continue;
				}

				IPXAddressClass* address = IPXConnectionAddress(ipx, connectionId);
				if (!address) {
					continue;
				}

				SendGlobalMessage_sub5410F0(ipx, packet, sizeof(*packet), 1, address, 0, 0);
				IPXService(ipx);
				++sentCount;
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::Log("SendChatPacketToConnections failed.");
	}

	return sentCount;
}

void SendChatMessage(const wchar_t* message) {
	ChatGlobalPacket packet = {};
	if (!BuildChatGlobalPacket(message, &packet)) {
		return;
	}

	const int sentCount = SendChatPacketToConnections(&packet);
	DisplayChatMessage(packet.Name, packet.PlayerColor, packet.Message);
	Utils::LogFormat(
		"Chat send: multiplayer=%d sent=%d color=%d crc=%d",
		SessionClass::IsMultiplayer() ? 1 : 0,
		sentCount,
		packet.PlayerColor,
		packet.NameCRC);
}

void PrintGameMessage(const wchar_t* message) {
	if (!message || !*message) {
		return;
	}

	__try {
		MessageListClass::Instance.PrintMessage(message);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
}

void Chat(const wchar_t* msg) {
	SendChatMessage(msg);
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
//	Patch::Apply_CALL(/* UDP send hook */, NetHack::SendTo);
//	Patch::Apply_CALL(/* UDP recv hook */, NetHack::RecvFrom);
//}
}

void Install(HMODULE hModule) {
	// 设置配置文件路径
	TCHAR szPath[MAX_PATH] = { 0 };
	Utils::GetStartPath(hModule, szPath, MAX_PATH);
	_tcscat_s(szPath, MAX_PATH, _T("Ra2Tool.ini"));
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

	if (SessionClass::IsMultiplayer()) {
		Utils::Log("游戏日志输出功能在多人模式下可能存在问题，本次不会开启，推荐在单人模式下使用。");
		return false; 
	}

	void* oldFunc = (byte*)GetModuleHandleA(0) + RA2_OFFSET_EMPTY_LOG_STUB; // RA2_FUNC_EMPTY_LOG_STUB 是一个空函数，就一个retn
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
			 mov eax, RA2_ADDR_CURRENT_HOUSE
			 mov eax, [eax]
			 mov eax, RA2_ADDR_MOUSE_CLASS
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
