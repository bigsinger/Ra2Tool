# Ra2Tool 开发文档

## 地址常量

本项目除 `YRpp` 目录外，所有正在使用的 RA2/YR 游戏地址统一声明在 `Ra2Tool/Ra2Header.h`。新增地址时先在该文件分组命名，再在业务代码中引用常量，不要直接写裸地址。

地址分组：

- 全局对象和数组：`RA2_ADDR_CURRENT_HOUSE`、`RA2_ADDR_MOUSE_CLASS`、`RA2_ADDR_SELECTED_OBJECTS`、`RA2_ADDR_COMMAND_BUTTON_LIST` 等。
- 函数入口：`RA2_FUNC_EVENT_TARGET_CTOR`、`RA2_FUNC_REVEAL_MAP`、`RA2_FUNC_SEND_GLOBAL_MESSAGE`、`RA2_FUNC_COLOR_ENUM` 等。
- VTable 和 Hook/Patch 点：`RA2_VTABLE_BUILDING`、`RA2_HOOK_OVERLAY_DRAW`、`RA2_PATCH_DEBUG_NEWLINE` 等。

`0x7FFFFFFF`、颜色掩码、float 位模式、结构偏移和机器指令 opcode 不属于游戏地址，保留在使用点。

## CommandBarButtons 配置

`Ra2Tool/CommandBarButtons.cpp` 使用三组静态配置数组创建底部 CommandBar 预设按钮：

```cpp
static const char* SHPFileNames[] = {
    "Button09.SHP",
    "Button10.SHP",
    "Button11.SHP"
};

static const char* CommandButtonTooltips[] = {
    "Button Tip 1",
    "Button Tip 2",
    "Button Tip 3"
};

static CommandButtonCallback CommandButtonCallbacks[] = {
    OnCommandButtonPreset1,
    OnCommandButtonPreset2,
    OnCommandButtonPreset3
};
```

实际创建数量取三组数组长度的最小值。新增按钮时同时增加 SHP、tooltip 和回调；如果三组长度不一致，多出的配置不会被创建。

点击响应逻辑在 `CheckCommandButtons()`，根据按钮索引调用 `CommandButtonCallbacks[index]()`。默认回调只输出日志。

## 巨炮辅助

`GrandCannonAssist.cpp` 负责：

- 己方巨炮建造完成后自动攻击范围内敌方巨炮。
- 无敌方巨炮时按配置角度自动转向。
- 敌方巨炮部署在己方巨炮射程内时自动攻击。
- 巨炮血量低于阈值时自动派工程师抢修。

巨炮建造完成判定使用 `IsGrandCannonReady()`，避免建筑仍处于 `BStateType::Construction` 时被提前加入已处理集合。

## 捡箱停止

单次捡箱由 `CrateAssist.cpp` 的 `g_autoPickup` 和 `g_pendingPickup` 管理。用户按 Stop 后：

- 检测 `CurrentMission` 或 `QueuedMission` 为 `Mission::Stop`。
- 清空自动捡箱主状态和 pending 冲箱状态。
- 对相关单位补发 `Mission::Stop`。
- 启动短时间 `StopHoldState`，连续补发 Stop，压掉可能已经排队的移动事件。

如需新增自动移动行为，必须明确在 Stop 时清理自己的 pending 状态。
