#pragma once

#include <YRPPCore.h>
#include <Interfaces.h>
#include <Helpers/CompileTime.h>

class DSurface;
struct RectangleStruct;
class GadgetClass;
class NOVTABLE GScreenClass : public IGameMap
{
public:
	//Static
	DEFINE_REFERENCE(GScreenClass, Instance, 0x87F7E8u)

	static void __fastcall DoBlit(bool mouseCaptured, DSurface* surface, RectangleStruct* rect = nullptr)
		{ JMP_STD(0x4F4780); }

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	virtual ULONG __stdcall AddRef() R0;
	virtual ULONG __stdcall Release() R0;

	//IGameMap

	//Destructor
	virtual ~GScreenClass() RX;

	//GScreenClass
	virtual void One_Time() RX;
	virtual void Init() RX;
	virtual void Init_Clear() RX;
	virtual void Init_IO() RX;
	virtual void GetInputAndUpdate(DWORD& outKeyCode, int& outMouseX, int& outMouseY) RX;
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) RX;
	virtual bool SetButtons(GadgetClass* pGadget) R0;
	virtual bool AddButton(GadgetClass* pGadget) R0;
	virtual bool RemoveButton(GadgetClass* pGadget) R0;
	virtual void MarkNeedsRedraw(int dwUnk) RX;
	virtual void DrawOnTop() RX;
	virtual void Draw(DWORD dwUnk) RX;
	virtual void vt_entry_44() RX;
	virtual bool SetCursor(MouseCursorType idxCursor, bool miniMap) = 0;
	virtual bool UpdateCursor(MouseCursorType idxCursor, bool miniMap) = 0;
	virtual bool RestoreCursor() = 0;
	virtual void UpdateCursorMinimapState(bool miniMap) = 0;

	void Render() { JMP_THIS(0x4F4480); }

protected:
	//Constuctor
	GScreenClass() {}	//don't need this

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	int ScreenShakeX;
	int ScreenShakeY;
	int Bitfield;	//default is 2
};
