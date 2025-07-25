#pragma once

#include <ArrayClasses.h>
#include <GeneralStructures.h>

#include <Helpers/CompileTime.h>

class SpotlightClass
{
public:
	//Static
	DEFINE_REFERENCE(DynamicVectorClass<SpotlightClass*>, Array, 0xAC1678u)

	//Destructor
	~SpotlightClass()
		{ THISCALL(0x5FF2D0); }

	void Draw()
		{ JMP_THIS(0x5FF850); }

	void Update()
		{ JMP_THIS(0x5FF320); }

	static void __fastcall DrawAll()
		{ JMP_STD(0x5FFFA0); }

	//Constructor
	SpotlightClass(CoordStruct coords, int size)
		{ JMP_THIS(0x5FF250); }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct Coords;
	int MovementRadius;
	int Size;
	SpotlightFlags DisableFlags;
};
