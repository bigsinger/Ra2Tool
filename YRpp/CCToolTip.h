#pragma once

#include <ToolTipManager.h>
#include <Drawing.h>

#include <Helpers/CompileTime.h>

class NOVTABLE CCToolTip : public ToolTipManager
{
public:
	// It's also used in MoneyFormat at 6A934A, not sure what side effect it might leads
	DEFINE_REFERENCE(bool, HideName, 0x884B8C)
	DEFINE_REFERENCE(CCToolTip*, Instance, 0x887368)
	DEFINE_REFERENCE(RGBClass, ToolTipTextColor, 0xB0FA1C)

	//Properties
public:
	bool FullRedraw;
	int Delay;
};
