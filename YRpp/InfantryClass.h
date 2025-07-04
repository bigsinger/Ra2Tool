/*
	Infantry
*/

#pragma once

#include <FootClass.h>
#include <InfantryTypeClass.h>

class NOVTABLE InfantryClass : public FootClass
{
public:
	static const AbstractType AbsID = AbstractType::Infantry;
	static constexpr uintptr_t AbsVTable = 0x7EB058;

	//Static
	DEFINE_REFERENCE(DynamicVectorClass<InfantryClass*>, Array, 0xA83DE8u)

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//Destructor
	virtual ~InfantryClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int	Size() const R0;

	//ObjectClass

	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const override JMP_THIS(0x51E3B0);

	//TechnoClass
	virtual FireError GetFireError(AbstractClass* pTarget, int nWeaponIndex, bool ignoreRange) const override JMP_THIS(0x51C8B0);


	//InfantryClass
	virtual bool IsDeployed() const R0;
	virtual bool PlayAnim(Sequence index, bool force = false, bool randomStartFrame = false) R0;

	//Constructor
	InfantryClass(InfantryTypeClass* pType, HouseClass* pOwner) noexcept
		: InfantryClass(noinit_t())
	{ JMP_THIS(0x517A50); }

protected:
	explicit __forceinline InfantryClass(noinit_t) noexcept
		: FootClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	InfantryTypeClass* Type;
	Sequence SequenceAnim; //which is currently playing
	CDTimerClass unknown_Timer_6C8;
	DWORD          PanicDurationLeft; // set in ReceiveDamage on panicky units
	bool           PermanentBerzerk; // set by script action, not cleared anywhere
	bool           Technician;
	bool           unknown_bool_6DA;
	bool           Crawling;
	bool           unknown_bool_6DC;
	bool           unknown_bool_6DD;
	DWORD          unknown_6E0;
	bool           ShouldDeploy;
	int            unknown_int_6E8;
	PROTECTED_PROPERTY(DWORD, unused_6EC); //??
};
