#pragma once

#include <ASMMacros.h>
#include <Helpers/CompileTime.h>

class GameOptionsClass
{
public:
	DEFINE_REFERENCE(GameOptionsClass, Instance, 0xA8EB60u)
	DEFINE_REFERENCE(bool, WindowedMode, 0x89F978u)

	int GetAnimSpeed(int rate)
		{ JMP_THIS(0x5FB2E0); }

	int GameSpeed;
	int Difficulty;
	int CampDifficulty;
	int ScrollMethod;
	int ScrollRate;
	bool AutoScroll;
	int DetailLevel;
	bool SidebarMode; // True -> right False -> left
	bool SidebarCameoText;
	bool UnitActionLines;
	bool ShowHidden;
	bool Tooltips;
	int ScreenWidth;
	int ScreenHeight;
	int ShellWidth;
	int ShellHeight;
	bool StretchMovies;
	bool AllowHiResModes;
	bool AllowVRAMSidebar;
	float SoundVolume;
	float VoiceVolume;
	float MovieVolume;
	bool IsScoreRepeat;
	bool InGameMusic;
	bool IsScoreShuffle;
	short SoundLatency;
	short Socket;
	int LastUnlockedSovMovie;
	int LastUnlockedAllMovie;
	int NetCard;
	char NetID[32];
	int unknown_int_78;
	int unknown_int_7C;
	int unknown_int_80;
	int unknown_int_84;
	int unknown_int_88;
	int unknown_int_8C;
	int unknown_int_90;
	int unknown_int_94;

	// virtual key constants, each of them doubled
	// defaulting to VK_MENU, VK_CONTROL and VK_SHIFT

	int KeyForceMove1;
	int KeyForceMove2;
	int KeyForceFire1;
	int KeyForceFire2;
	int KeyForceSelect1;
	int KeyForceSelect2;

	int KeyQueueMove1;
	int KeyQueueMove2;
};
