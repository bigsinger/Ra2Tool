#pragma once

#include <YRPP.h>

#include <Helpers/CompileTime.h>

struct SHPStruct;
class Surface;

class LoadProgressManager
{
public:
	static constexpr reference<LoadProgressManager*, 0xABC9BCu> const Instance{};

	LoadProgressManager()
		{ JMP_THIS(0x552A40); }

	virtual ~LoadProgressManager()
		{ JMP_THIS(0x552AA0); }

	void Draw()
		{ JMP_THIS(0x552D60); }

	DWORD field_4;
	DWORD field_8;
	RectangleStruct TitleBarRect;
	RectangleStruct BackgroundRect;
	RectangleStruct LoadBarRect;
	wchar_t* LoadMessage;
	wchar_t* LoadBriefing;
	SHPStruct * TitleBarSHP;
	SHPStruct * LoadBarSHP;
	SHPStruct * BackgroundSHP;
	bool TitleBarSHP_loaded;
	bool LoadBarSHP_loaded;
	bool BackgroundSHP_loaded;
	DWORD field_54;
	DWORD field_58;
	DWORD field_5C;
	DSurface * ProgressSurface;
};
