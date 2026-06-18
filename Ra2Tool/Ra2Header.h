#pragma once
#include <windows.h>
#include <stdint.h>


//#define DEVDEBUG

// RA2/YR runtime addresses used by this plugin. Keep raw game addresses here so
// call sites can show intent instead of repeating magic numbers.

enum : DWORD {
	// Global object pointers / arrays.
	RA2_ADDR_CURRENT_HOUSE = 0x00A83D4C,          // HouseClass* current house pointer storage.
	RA2_ADDR_MOUSE_CLASS = 0x0087F7E8,            // MouseClass / display singleton used by several UI calls.
	RA2_ADDR_SCENARIO_CLASS = 0x00A8B230,         // ScenarioClass::Instance pointer storage.
	RA2_ADDR_SELECTED_OBJECTS = 0x00A8ECBC,       // TechnoClass** selected object list pointer.
	RA2_ADDR_SELECTED_COUNT = 0x00A8ECC8,         // Selected object count.
	RA2_ADDR_BEACON_MANAGER = 0x0089C3B0,         // Beacon manager object.
	RA2_ADDR_COMMAND_BUTTON_LIST = 0x00B0C1C0,    // ShapeButtonClass command button object array.
	RA2_ADDR_COMMAND_BUTTON_INDEX = 0x00B0CB78,   // Visible command button slot-to-object index array.
	RA2_ADDR_OVERLAY_SURFACE = 0x00A8B8B4,        // Surface pointer restored by legacy overlay hook.

	// Function entry points.
	RA2_IMAGE_BASE = 0x00400000,                  // Default RA2/YR image base.
	RA2_FUNC_EVENT_TARGET_CTOR = 0x004C65E0,      // EventClass target constructor.
	RA2_FUNC_REVEAL_MAP = 0x00577D90,             // Reveal whole tactical map call.
	RA2_FUNC_REVEAL_TECH = 0x006A6300,            // Reveal tech/cameo helper call.
	RA2_FUNC_CLEAR_BEACON_ONE = 0x004311C0,       // Remove one beacon.
	RA2_FUNC_CLEAR_BEACONS_BY_HOUSE = 0x00431410, // Remove beacons for a house.
	RA2_FUNC_CRATE_METHOD_TABLE = 0x004833C4,     // Crate effect dispatch table.
	RA2_FUNC_CRATE_MONEY_EFFECT = 0x00482463,     // Crate money effect handler.
	RA2_FUNC_SEND_GLOBAL_MESSAGE = 0x005410F0,    // IPXManagerClass global message send.
	RA2_FUNC_COMPUTE_NAME_CRC = 0x005DAC00,       // Multiplayer name CRC routine.
	RA2_FUNC_IPX_NUM_CONNECTIONS = 0x00540F90,    // IPX connection count.
	RA2_FUNC_IPX_CONNECTION_ID = 0x00540FA0,      // IPX connection id lookup.
	RA2_FUNC_IPX_CONNECTION_NAME = 0x00540FC0,    // IPX connection player name lookup.
	RA2_FUNC_IPX_CONNECTION_ADDRESS = 0x00541000, // IPX connection address lookup.
	RA2_FUNC_IPX_SERVICE = 0x00541820,            // IPX service flush.
	RA2_FUNC_COLOR_ENUM = 0x0069A310,             // Chat color enum conversion.
	RA2_FUNC_EMPTY_LOG_STUB = 0x004068E0,         // Empty retn function used by the optional log hook.
	RA2_OFFSET_EMPTY_LOG_STUB = RA2_FUNC_EMPTY_LOG_STUB - RA2_IMAGE_BASE,

	// VTables / patch points.
	RA2_VTABLE_BUILDING = 0x007E3EBC,             // BuildingClass vtable.
	RA2_HOOK_OVERLAY_DRAW = 0x004F4573,           // Legacy overlay draw hook address.
	RA2_HOOK_OVERLAY_DRAW_RETURN = 0x004F4578,    // Return address after legacy overlay hook.
	RA2_PATCH_DEBUG_NEWLINE = 0x00825F9B,         // Debug-log newline patch point.
	RA2_PATCH_DEBUG_RA2MD_INI = 0x008332F4,       // Debug-log SUN.INI text patch point.
};

const int CRATE_MAX_COUNT = 0x100;	// 最大箱子数量
const int CRATE_LABEL_WIDTH = 60;   // 箱子标签宽度
const int CRATE_LABEL_HEIGHT = 40;  // 箱子标签高度


//////////////////////////
extern HINSTANCE g_thisModule;      // 当前模块句柄
extern RECT gameClientRect;         // 游戏客户端矩形区域
extern POINT gameClientTopLeft;     // 游戏客户端左上角坐标
extern HWND g_hwndTipWindow;
//////////////////////////


enum NetCommandType {
    NET_QUERY_GAME = 0x0,
    NET_ANSWER_GAME = 0x1,
    NET_QUERY_PLAYER = 0x2,
    NET_ANSWER_PLAYER = 0x3,
    NET_CHAT_ANNOUNCE = 0x4,
    NET_CHAT_REQUEST = 0x5,
    NET_QUERY_JOIN = 0x6,
    NET_CONFIRM_JOIN = 0x7,
    NET_REJECT_JOIN = 0x8,
    NET_GAME_OPTIONS = 0x9,
    NET_SIGN_OFF = 0xA,
    NET_GO = 0xB,
    NET_MESSAGE = 0xC,
    NET_PING = 0xD,
    NET_LOADGAME = 0xE,
    NET_ProgressLoading = 0xF,
    NET_Taunts = 0x1D,
    NET_BeaconPlace = 0x20,
    NET_BeaconDelete = 0x21,
    NET_BeaconSendText = 0x22,
    NET_REQ_SCENARIO = 0x3E8,
    NET_FILE_INFO = 0x3E9,
    NET_FILE_CHUNK = 0x3EA,
    NET_READY_TO_GO = 0x3EB,
    NET_NO_SCENARIO = 0x3EC,
};

#pragma pack(push, 1)
struct GlobalPacketType {
    NetCommandType Command;
    int16_t Name[20];
    union {
        struct {
            char Data[411];  // 0x1C3
        } SpaceGap;
        struct {
            char Data[3];
            int Progress;
        } ProgressLoading;
        struct {
            char Data[3];
            bool Unknown;
            char Buf[224];
            int PlayerColor;
            int NameCRC;
        } Message;
        struct {
            char Data[3];
            int GameStart;
        } StartGame;
        struct {
            char Data[3];
            bool Unknown;
            int TauntsIndex;
        } Taunts;
        struct {
            char Data[3];
            int CoordX;
            int CoordY;
            int CoordZ;
            char Index;
            char PlayerIndex;
        } BeaconPlaceDelete;
        struct {
            char Data[3];
            char index;
            char player_idx;
            char Text[255];
        } BeaconSendText;
    } GlobalPacketData;
};
#pragma pack(pop)
static_assert(sizeof(GlobalPacketType) == 455);

