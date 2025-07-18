/*
	Aircraft
*/

#pragma once

#include <FootClass.h>
#include <AircraftTypeClass.h>

//AircraftClass
class NOVTABLE AircraftClass : public FootClass, public IFlyControl
{
public:
	static const AbstractType AbsID = AbstractType::Aircraft;
	static constexpr uintptr_t AbsVTable = 0x7E22A4;

	//Static
	DEFINE_REFERENCE(DynamicVectorClass<AircraftClass*>, Array, 0xA8E390u)

	//IFlyControl
	virtual int __stdcall Landing_Altitude() R0;
	virtual int __stdcall Landing_Direction() R0;
	virtual long __stdcall Is_Loaded() R0;
	virtual long __stdcall Is_Strafe() R0;
	virtual long __stdcall Is_Fighter() R0;
	virtual long __stdcall Is_Locked() R0;

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	virtual ULONG __stdcall AddRef() R0;
	virtual ULONG __stdcall Release() R0;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int	Size() const R0;

	//Destructor
	virtual ~AircraftClass() RX;

	//Constructor
	AircraftClass(AircraftTypeClass* pType, HouseClass* pOwner) noexcept
		: AircraftClass(noinit_t())
	{ JMP_THIS(0x413D20); }

	AbstractClass* FindFireLocation(AbstractClass* pTarget)
		{ JMP_THIS(0x4197C0); }

protected:
	explicit __forceinline AircraftClass(noinit_t) noexcept
		: FootClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	AircraftTypeClass* Type;
	bool ShouldLoseAmmo; // Whether or not to deduct ammo after firing run (strafing) is over
	bool HasPassengers;	//parachutes
	bool IsKamikaze; // when crashing down, duh
	BuildingClass* DockNowHeadingTo;
	bool unknown_bool_6D0;
	bool unknown_bool_6D1;
	bool IsLocked; // Whether or not aircraft is locked to a firing run (strafing)
	char NumParadropsLeft;
	bool IsCarryallNotLanding;
	bool IsReturningFromAttackRun; // Aircraft finished attack run and/or went idle and is now returning from it
};
