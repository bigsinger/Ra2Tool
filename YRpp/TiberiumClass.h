/*
	Tiberiums are initialized by INI files.
*/

#pragma once

#include <AbstractTypeClass.h>
#include <PriorityQueueClass.h>

//forward declarations
class AnimTypeClass;
class OverlayTypeClass;

class TiberiumLogic
{
public:
	void Construct(int nCount = PriorityQueueClassNode::SurfaceDataCount())
	{
		Nodes = (PriorityQueueClassNode*)YRMemory::Allocate(sizeof(PriorityQueueClassNode) * nCount);
		CellIndexesWithTiberium = (bool*)YRMemory::Allocate(sizeof(bool) * nCount);

		Queue = GameCreate<PriorityQueueClass<PriorityQueueClassNode>>(nCount);
	}

	void Destruct()
	{
		GameDelete(Queue);
		Queue = nullptr;

		if (Nodes)
		{
			YRMemory::Deallocate(Nodes);
			Nodes = nullptr;
		}

		if (CellIndexesWithTiberium)
		{
			YRMemory::Deallocate(CellIndexesWithTiberium);
			CellIndexesWithTiberium = nullptr;
		}
	}

	int Count;
	PriorityQueueClass<PriorityQueueClassNode>* Queue;
	bool* CellIndexesWithTiberium;
	PriorityQueueClassNode* Nodes;
	CDTimerClass Timer;
};

class NOVTABLE TiberiumClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::Tiberium;

	//Array
	ABSTRACTTYPE_ARRAY(TiberiumClass, 0xB0F4E8u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;
	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) R0;

	//Destructor
	virtual ~TiberiumClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//TiberiumClass

	void RegisterForGrowth(CellStruct* cell)
		{ JMP_THIS(0x7235A0); }

	//Static helpers

	static int FindIndex(int idxOverlayType) {
		SET_REG32(ecx, idxOverlayType);
		CALL(0x5FDD20);
	}

	static TiberiumClass* Find(int idxOverlayType) {
		int idx = FindIndex(idxOverlayType);
		return Array.GetItemOrDefault(idx);
	}

	//Constructor
	TiberiumClass(const char* pID)
		: TiberiumClass(noinit_t())
	{ JMP_THIS(0x7216C0); }

protected:
	explicit __forceinline TiberiumClass(noinit_t)
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	int Spread;
	double SpreadPercentage;
	int Growth;
	double GrowthPercentage;
	int Value;
	int Power;
	int Color;
	DECLARE_PROPERTY(TypeList<AnimTypeClass*>, Debris);
	OverlayTypeClass* Image;
	int NumFrames;
	int NumImages;
	int NumSlopes;
	DECLARE_PROPERTY(TiberiumLogic, SpreadLogic);
	DECLARE_PROPERTY(TiberiumLogic, GrowthLogic);
};
