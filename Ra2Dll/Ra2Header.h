#pragma once
#include <stdint.h>


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

