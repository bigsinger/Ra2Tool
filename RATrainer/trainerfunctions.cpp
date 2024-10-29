#include "windows.h"
#include "trainerfunctions.h"
#include <stdio.h>

//////////////////////////////////////////////�Ĵ���
BOOL TrainerFunctions::rSpeed(DWORD* pSpeed) //��ȡ�ٶ�
{
	const DWORD address = 0x00A8EB60;
	if(!readMemory(address,pSpeed))
		return FALSE;
	return TRUE;
}

BOOL TrainerFunctions::wSpeed(DWORD Speed) //д���ٶ�
{
	const DWORD address = 0x00A8EB60;
	DWORD SpeedTest;
	if(!readMemory(address,&SpeedTest))
		return FALSE;
	writeMemory(address,Speed);
	return TRUE;
}

BOOL TrainerFunctions::rMymoney(DWORD* pMoney) //��ȡ��Ǯ
{
	const DWORD address[2] = {0x00A83D4C,0x30C};
	DWORD money;
	if(!readMemory(address,2,&money))
		return FALSE;
	*pMoney = money;
	return TRUE;
}

BOOL TrainerFunctions::wMymoney(DWORD money) //��Ǯ�޸�
{
	const DWORD address[2] = {0x00A83D4C,0x30C};
	DWORD add;
	if(!readAddress(address,2,&add))
		return FALSE;
	writeMemory(add,money); //ʵ�ʽ�Ǯ��ַ
	return TRUE;
}


BOOL TrainerFunctions::rBuildMaxm(DWORD* pMaxm) //��ȡ�����������
{
	const DWORD address[2] = {0x008871E0,0xF0};
	DWORD maxm;
	if(!readMemory(address,2,&maxm))
		return FALSE;
	*pMaxm = maxm+1;
	return TRUE;
}

BOOL TrainerFunctions::wBuildMaxm(DWORD maxm) //�����޸�
{
	const DWORD address[2] = {0x008871E0,0xF0};
	DWORD add;
	if(!readAddress(address,2,&add))
		return FALSE;
	writeMemory(add,maxm-1); //
	return TRUE;
}

BOOL TrainerFunctions::QuickBuild() //���ٽ���
{
	int j=0;
	DWORD dat;
	if(!readMemory(0x00A83D4C,&dat))
		return FALSE;
	for(int i=1;i<=5;(j+=4),i++)
	{
		writeMemory(dat+j+0x5378,15);
	}
	return TRUE;
}

BOOL TrainerFunctions::RadarOn() //�������״�
{
	const DWORD address[2] = {0x00A8B230,0x34A4};
	DWORD add;
	if(!readAddress(address,2,&add))
		return FALSE;
	writeMemory(add,0x1,0x1);
	return TRUE;
}

BOOL TrainerFunctions::SuperOn() //���޳���
{
	//00 NuclearMissile
	//01 IronCurtain
	//02 ForceShield
	//03 LightningStorm
	//04 PsychicDominator
	//05 Chronosphere
	//06 Reinforcements
	//07 SpyPlane
	//08 GeneticMutator
	//09 PsychicReveal
	//����һ��11������
	const DWORD add_pre[2] = {0x00A83D4C,0x258};
	DWORD t1,t2,dis;	//dis->0~54

	if(!readMemory(add_pre,2,&t1)){return FALSE;}
	for(DWORD i=0; i<12; i++)		//11��ѭ��
	{
		if(!readMemory(t1+i*4,&t2)){return FALSE;}
		if(!readMemory(t2+0x78,&dis)){return FALSE;}	//��ȡ�����ʾ��û����-1
		if((long)dis != -1)
			writeMemory(t2+0x6F,0x1,0x1);		//�޸ľ�����ʶ
	}
	return TRUE;
}

void TrainerFunctions::WinImme() //����ʤ��
{
	writeMemory(0x00A83D49,0x1,0x1);
}

BOOL TrainerFunctions::TobeGhost() //��Ϊ�������
{
	DWORD dat1,dat2;
	readMemory(0x00A8ECC8,&dat1);	//ѡ������
	if(dat1 != 1)		//����ѡ��һ������
		return FALSE;

	DWORD address[3] = {0x00A8ECBC,0,0};
	readMemory(address,3,&dat1);		//ѡ�е�λ�׵�ַ
	if(dat1 != 0x007E3EBC)		//����ѡ��һ������
		return FALSE;

	address[2] = 0x21C;		//��λ����
	readMemory(address,3,&dat1);		//ѡ�е�λ����
	readMemory(0x00A83D4C,&dat2);		//������ݻ�ַ
	if(dat1 != dat2)		//����ѡ����ҵ�λ
		return FALSE;
	//����������ʼת��
	DWORD PlayerID[2] = {0x00A83D4C,0x30};
	readMemory(PlayerID,2,&dat1);		//��ȡ��ҵ�ǰID
	if(dat1 != 0)		//ת�Ƶ�����ID
		dat1--;
	else
		dat1++;

	DWORD AimID[2] = {0x00A8022C,4*dat1};	//Ŀ����һ�ַ
	readMemory(AimID,2,&dat2);		//��ȡĿ��������ݻ�ַ
	readAddress(address,3,&dat1);		//ѡ�е�λ�����洢��ַ
	writeMemory(dat1,dat2);		//ת��

	//address[2] = 0x6C;		//��λѪ��
	//readAddress(address,3,&dat1);		//ѡ�е�λѪ���洢��ַ
	//writeMemory(dat1,0);		//����ѡ�еĽ�����

	//ɾ��ѡ�е�λ
	writProcess(DeleteThis_Assemble);

	return TRUE;	
}

//////////////////////////////////////////////////�߳�
//ȫ��ͼ��������
#pragma pack(1)
void TrainerFunctions::Map_Assemble() {
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
void TrainerFunctions::RadarOn_Assemble() {
	_asm {
		pushad
		mov eax, 0x00A8B230
		mov eax, [eax]
		mov byte ptr [eax + 0x34A4], 0x1
		popad
	}
}

// ���м�����Ч������Ǯ
void TrainerFunctions::SetBoxAllMoney() {
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
	char buff1[1024] = {};
	char buff2[1024] = {};
	sprintf_s(buff1, Format, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	sprintf_s(buff2, "[Ra2] %s", buff1);
	OutputDebugStringA(buff2);
}

byte backupCode[5] = {};
bool TrainerFunctions::OpenLog() {
	OutputDebugStringA("[Ra2] OpenLog");

	void* oldFunc = (byte*)GetModuleHandleA(0) + 0x68E0; // 0x4068E0 ��һ���պ�������һ��retn
	memcpy(&backupCode, oldFunc, 5u);
	return installHook(oldFunc, (DWORD)&log);
}
#pragma optimize("", on)


// �ж���Ϸ�Ƿ�����
 bool TrainerFunctions::isGameRunning() {
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

void TrainerFunctions::AllMap() {
	//��ͼȫ��
	writProcess(Map_Assemble);
}



//����˵���������
void TrainerFunctions::NuclearBomb_Assemble()
{
	int eax_temp;       
	__asm
	{
		pushad
		mov ecx,0x00A83D4C	//����Ƿ��Ѿ��к˵����侮
		mov ecx,[ecx]
		add ecx,0x258
		mov ecx,[ecx]
		mov ecx,[ecx]
		add ecx,0x78
		mov ecx,[ecx]
		cmp ecx,0xFFFFFFFF	//���-1��ʾû�� 0~54��ʾ��
		jne exit1	//�о��˳���ֹ����

		mov ecx,0x14
		mov eax_temp,0x006CEEB0
		call eax_temp
		mov ebx,eax
		//�����˵�CD
		add ebx,0x98
		mov eax,[ebx]//������
		mov edx,0x00A83D4C
		mov edx,[edx]
		add edx,0x258
		mov ecx,[edx]
		push 0           //��ʼ״̬��1=�ȴ�
		push 0           //��Ӧ����
		mov ecx,[ecx+eax*4]
		push 1             //1=һ���� �˵�����1 ��������0
		mov eax_temp,0x006CB560
		call eax_temp
		//���Ӻ˵�����ѡ��
		add ebx,0x98
		mov eax,[ebx]//������
		mov ecx,0x0087F7E8
		push eax
		push 0x1F
		mov eax_temp,0x006A6300
		call eax_temp

		exit1:
		popad             
	}       
}

void TrainerFunctions::GetaNuclearBomb()
{
	//����˵�
	writProcess(NuclearBomb_Assemble);
}

//��λ�����������
void TrainerFunctions::UpChose_Assemble()
{
	__asm
	{
		pushad
		mov eax,0x00A8ECBC
		mov ebx,[eax]	//ѡ�е�λ�׵�ַ
		mov eax,0

		process:
		mov ecx,0x00A8ECC8
		cmp eax,[ecx]  //ѡ�е�λ����Ϊ���ж�
		jge brek//>=��ת
		mov ecx,[ebx+eax*4]  //��ǰ��λ��ַ
		mov edx,0x40000000		//Float 2.0
		add ecx,0x150
		mov [ecx],edx  //��ǰ��λ����
		add eax,1
		jmp process

		brek:
		popad
	}
}

void TrainerFunctions::UpChose()
{
	//��λ����
	writProcess(UpChose_Assemble);
}

//��λ�����������
void TrainerFunctions::SpeedUpChose_Assemble()
{
	__asm
	{
		pushad
		mov eax,0x00A8ECBC
		mov ebx,[eax]	//ѡ�е�λ�׵�ַ
		mov eax,0

		process:
		mov ecx,0x00A8ECC8
		cmp eax,[ecx]  //ѡ�е�λ����
		jge brek//>=��ת
		mov ecx,[ebx+eax*4]  //��ǰ��λ��ַ
		mov edx,0x007E3EBC
		cmp [ecx],edx	//��ǰ��λ����Ϊ����
		je brek			//����û�ٶȲ��ܸ�
		mov edx,0x007EB058
		cmp [ecx],edx	//��������2.0
		jne next1
		mov edx,0x40000000
		add ecx,0x584
		mov [ecx],edx
		je loopcontinue
		next1:
		mov edx,0x007F5C70
		cmp [ecx],edx	//����
		jne next2
		mov edx,0x40A00000
		add ecx,0x584
		mov [ecx],edx	//5.0
		je loopcontinue
		next2:
		//�ɻ������û��
		//mov [ecx+584],3FF00000//1.875
		loopcontinue:
		add eax,1
		jmp process

		brek:
		popad
	}
}

void TrainerFunctions::SpeedUpChose()
{
	//��λ����
	writProcess(SpeedUpChose_Assemble);
}

//ȫ���ҵ�-ѡ�е�λ �������
void TrainerFunctions::MineChose_Assemble()
{
	__asm
	{
		pushad
		mov eax,0x00A8ECC8	//ѡ�е�λ����
		mov eax,[eax]
		cmp eax,0		//�Ƿ�ѡ�е�λ
		je exit1
		push 0  //
		mov ebx,0x00A83D4C
		mov eax,[ebx]
		push eax
		mov ebx,0x00A8ECBC
		mov eax,[ebx]
		mov ecx,[eax]
		mov ebx,[ecx]
		add ebx,0x3D4
		mov ebx,[ebx]
		call ebx
		exit1:
		popad
	}
}

void TrainerFunctions::MineChose()
{
	//ȫ���ҵ�-ѡ�е�λ
	writProcess(MineChose_Assemble);
}

//ɾ��ѡ�е�λ �������
void TrainerFunctions::DeleteThis_Assemble()
{
	__asm
	{
		pushad
		mov eax,0x00A8ECC8
		mov eax,[eax]//ѡ�е�λ����
		cmp eax,0		//�Ƿ�ѡ�е�λ
		je exit1

		mov eax,0x00A8ECBC
		mov eax,[eax]
		mov ecx,[eax]
		mov eax,[ecx]
		add eax,0xF8
		mov ebx,[eax]
		call ebx

		exit1:
		popad
	}
}

void TrainerFunctions::DeleteThis()
{
	//ɾ��ѡ�е�λ
	writProcess(DeleteThis_Assemble);
}
