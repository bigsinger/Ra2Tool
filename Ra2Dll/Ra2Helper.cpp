#include "windows.h"
#include <stdio.h>
#include "Ra2Helper.h"


// ��ӡ������Ϣ
void printLastError(DWORD error, const char* tag/* = NULL*/) {
	char buffer[1024] = {};
	sprintf_s(buffer, sizeof(buffer), "Tag: %s, Error Code: %lu", tag, error);
	OutputDebugStringA(buffer);
}

//////////////////////////////////////////////////


//ȫ��ͼ��������
#pragma pack(1)
void OpenMap() {
	_asm {
		pushad
		mov eax, 0x00A83D4C
		mov edx, [eax]
		mov ecx, 0x0087F7E8
		push edx
		mov eax,0x00577D90
		call eax
		popad
	}
}

// �״�ȫ�� [[0x00A8B230] + 0x34A4] = 1
void OpenRadar() {
	_asm {
		pushad
		mov eax, 0x00A8B230
		mov eax, [eax]
		mov byte ptr [eax + 0x34A4], 0x1
		popad
	}
}

void _openTechOne(int objType, int index) {
	_asm {
		pushad
		mov ecx, 0x0087F7E8
		push index
		push objType
		mov eax, 0x006A6300
		call eax
		popad
	}
}

void _openTechAll(int objType, int count) {
	_asm {
		pushad
		xor esi, esi
	label:
		push esi
		push objType			// 0x7�ǽ�����0x10(Ҳ������0xF)�ǲ�����0x28�ǳ�����0x1F=����
		mov ecx, 0x0087F7E8
		mov eax, 0x006A6300
		call eax
		inc esi
		cmp esi, count			// ������һ���������ֵ���ο�����ini
		jle label
		popad
	}
}

// �Ƽ�ȫ��
void OpenTech() {
	//_openTechAll(7, 69);				// ����ȫ��
	//_openTechAll(0x10, 30);			// ����ȫ��
	//_openTechAll(0x28, 40);			// ̹��ȫ��

	_openTechOne(0x28, 0);		// �˾����س�
	_openTechOne(0x28, 26);	// �վ����س�
	//_openTechOne(0x28, 54);			// ������س�

	_openTechOne(0x28, 2);		// ����
	//_openTechOne(0x28, 16);			// �ֲ������ˣ�֩�룩
	//_openTechOne(0x28, 35);			// ����̹��
	_openTechOne(0x28, 36);	// ��Ӱ
	_openTechOne(0x28, 37);	// �๦��
}

// ��������̽��
void OpenPsychicDetection() {
	_asm {
		pushad
		mov eax, 0x00A8ECC8
		mov eax, [eax]				//ѡ�е�λ������
		cmp eax, 0					//�Ƿ���ѡ�е�λ
		je exit1
		mov eax, 0x00A8ECBC
		mov eax, [eax]
		mov eax, [eax]				// +0
		mov eax, [eax]				// +0
		cmp eax, 0x007E3EBC			// Building vt
		jnz exit1
		mov eax, 0x00A8ECBC
		mov eax, [eax]
		mov eax, [eax]				// +0
		mov eax, [eax + 0x520]		// +0x520
		mov word ptr[eax + 0x170C], 0x7FFF	// 0x170C PsychicDetectionRadius
	exit1:
		popad
	}
}

// ���м�����Ч������Ǯ
void SetBoxAllMoney() {
	const LPVOID MethodTableAddr = (LPVOID)0x004833C4;
	const size_t MethodTableCount = 0x12;
	const SIZE_T MethodTableSize = MethodTableCount * sizeof(DWORD);
	const DWORD JumpToAddr = 0x00482463;	// �񵽵��ǽ�Ǯ����ת��ַ

	// �޸Ĵ��뱣������
	DWORD dwOldProtect = 0;
	if (VirtualProtect(MethodTableAddr, MethodTableSize, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {

		DWORD* p = (DWORD*)MethodTableAddr;
		for (size_t i = 0; i < MethodTableCount; i++) {
			*(p + i) = JumpToAddr;
		}

		// �ָ����뱣������
		if (!VirtualProtect(MethodTableAddr, MethodTableSize, dwOldProtect, &dwOldProtect)) {
			printLastError(GetLastError(), "Failed 2!");
		}
	} else {
		printLastError(GetLastError(), "Failed 1!");
	}
}

#pragma optimize("", off)
BOOL installHook(LPVOID addr, DWORD newFunc) {
	byte buff[5];
	DWORD* offset = (DWORD*) & buff[1];
	DWORD dwWrittenSize = 0;
	DWORD dwOldProtect = 0;

	buff[0] = 0xE9;	// jmp
	*offset = newFunc - (DWORD)addr - 5;

	if (VirtualProtect(addr, 5u, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
		WriteProcessMemory(GetCurrentProcess(), addr, &buff, 5u, &dwWrittenSize);
		if (!VirtualProtect(addr, 5u, dwOldProtect, &dwOldProtect)) {
			printLastError(GetLastError(), "Failed 2!");
		}
	} else {
		printLastError(GetLastError(), "Failed 1!");
	}

	return true;	
}

void __cdecl log(const char* Format, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9) {
	if (!strstr(Format, "Theme")) {
		char buff1[1024] = {};
		char buff2[1024] = {};
		DWORD* pArgs = (DWORD*)((DWORD)&Format - 4);	//&p�ǲ���ջ�ĵ�ַ��&p-4�Ƿ��ص�ַ

		sprintf_s(buff1, Format, a1, a2, a3, a4, a5, a6, a7, a8, a9);
		sprintf_s(buff2, "[Ra2] %08X %s", pArgs[0], buff1);
		OutputDebugStringA(buff2);
	}
}

byte backupCode[5] = {};
bool OpenLog() {
	OutputDebugStringA("[Ra2] OpenLog");

	void* oldFunc = (byte*)GetModuleHandleA(0) + 0x68E0; // 0x4068E0 ��һ���պ�������һ��retn
	memcpy(&backupCode, oldFunc, 5u);
	return installHook(oldFunc, (DWORD)&log);
}
#pragma optimize("", on)


// �ж���Ϸ�Ƿ�����
 bool IsGameRunning() {
	 bool isRunning = false;

	 __try {
		 _asm {
			 pushad
			 mov eax, 0x00A83D4C
			 mov eax, [eax]
			 mov eax, 0x0087F7E8
			 mov eax, [eax]
			 popad
		 }

		 isRunning = true;
	 } __except (EXCEPTION_EXECUTE_HANDLER) {
		 isRunning = false;
	 }
	 return isRunning;
}

#pragma pack()
