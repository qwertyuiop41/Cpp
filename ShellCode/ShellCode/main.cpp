#include<Windows.h>
#include<iostream>
#include "CPeUtil.h"


//Ϊʲô��Ҫ����һ�飿
//������Ļ����޷�ʹ��PACKINFO����Ϣ
//�������ᱨ��δ�����ʶ�� PACKINFO������
typedef struct _PACKINFO {
	DWORD newOEP;
	DWORD oldOEP;
}PACKINFO, * PPACKINFO;


int main() {
	CPeUtil peUtil;
 	BOOL bSuccess = peUtil.LoadFile("FateMouse_upx.exe");
	
	peUtil.EncodeSections();
	HMODULE hModule = LoadLibraryA("pack.dll");
	// GetProcAddress �������Ƿ��ط��ŵ�ַ
	PPACKINFO pPackInfo = (PPACKINFO)GetProcAddress(hModule, "g_PackInfo");
	pPackInfo->oldOEP = peUtil.GetOldOEP();
	PIMAGE_DOS_HEADER pDosHeaderdll = (PIMAGE_DOS_HEADER)hModule;
	//Ҫ��hModule�ĳ�(DWORD)hModule����Ȼ�ᱨ��
	// ֱ��(HMODULE + DWORD) �ᵼ�±�������Ϊ��Ҫ�� sizeof(HMODULE) Ϊ��λ����ָ��ƫ�ƣ��������ֽ�ƫ�ƣ�
	PIMAGE_NT_HEADERS pNtHeaderdll = (PIMAGE_NT_HEADERS)((DWORD)hModule + pDosHeaderdll->e_lfanew);
	PIMAGE_SECTION_HEADER pSectionHeaderdll = IMAGE_FIRST_SECTION(pNtHeaderdll);
	char* sectionBuff = (char*)((DWORD)pSectionHeaderdll->VirtualAddress + hModule);

	BOOL bInsert = peUtil.InsertSection("51hook",pSectionHeaderdll->SizeOfRawData, sectionBuff,0xE00000E0);
	peUtil.RepairReloc((DWORD)hModule);
	DWORD OEPRVA = pPackInfo->newOEP - (DWORD)hModule;
	DWORD offset = OEPRVA - (DWORD)pSectionHeaderdll->VirtualAddress;
	BOOL bSet = peUtil.SetOEP(offset);
	peUtil.SaveFile("pack.exe");
	return 0;
}