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
	char* sectionBuff = (char*)(pSectionHeaderdll->VirtualAddress + (DWORD)hModule);
	BOOL bInsert = peUtil.InsertSection("51hook",pSectionHeaderdll->Misc.VirtualSize, sectionBuff,0xE00000E0);

	peUtil.RepairReloc((DWORD)hModule);
	DWORD OEPRVA = pPackInfo->newOEP - (DWORD)hModule;
	DWORD offset = OEPRVA - pSectionHeaderdll->VirtualAddress;
	BOOL bSet = peUtil.SetOEP(offset);
	peUtil.SaveFile("customPack.exe");
	return 0;
}

//ע�⣺
//����ͨ������ķ�ʽ���entry point
//��Ϊentry pointָ����DLL�ı�׼��ڵ㣨ͨ���� DllMain ����������ɵĳ�ʼ�����룩
//�Ǵ������ڵ㣡��ִ��DLL��ʼ����������ǿǴ���
//DWORD dllOEP = pNtHeaderdll->OptionalHeader.AddressOfEntryPoint;
//DWORD packStartOffset = dllOEP - pSectionHeaderdll->VirtualAddress;


//int main() {
//	CPeUtil peUtil;
//	BOOL bSuccess = peUtil.LoadFile("FateMouse_upx.exe");
//	HMODULE hModule = LoadLibraryA("pack.dll");
//	// GetProcAddress �������Ƿ��ط��ŵ�ַ
//	PPACKINFO pPackInfo = (PPACKINFO)GetProcAddress(hModule, "g_PackInfo");
//	pPackInfo->oldOEP = peUtil.GetOldOEP();
//	PIMAGE_DOS_HEADER pDosHeaderdll = (PIMAGE_DOS_HEADER)hModule;
//	//Ҫ��hModule�ĳ�(DWORD)hModule����Ȼ�ᱨ��
//	// ֱ��(HMODULE + DWORD) �ᵼ�±�������Ϊ��Ҫ�� sizeof(HMODULE) Ϊ��λ����ָ��ƫ�ƣ��������ֽ�ƫ�ƣ�
//	PIMAGE_NT_HEADERS pNtHeaderdll = (PIMAGE_NT_HEADERS)((DWORD)hModule + pDosHeaderdll->e_lfanew);
//	PIMAGE_SECTION_HEADER pSectionHeaderdll = IMAGE_FIRST_SECTION(pNtHeaderdll);
//	char* sectionBuff = (char*)(pSectionHeaderdll->VirtualAddress + (DWORD)hModule);
//
//	BOOL bInsert = peUtil.InsertSection("51hook", pSectionHeaderdll->Misc.VirtualSize, sectionBuff, 0xE00000E0);
//
//	peUtil.SaveFile("customPack2.exe");
//	return 0;
//}