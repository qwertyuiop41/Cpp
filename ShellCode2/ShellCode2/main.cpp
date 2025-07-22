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
	//peUtil.GetImportTable();
	
	PIMAGE_DOS_HEADER pDosHeaderdll = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNtHeaderdll = (PIMAGE_NT_HEADERS)((DWORD)hModule + pDosHeaderdll->e_lfanew);
	PIMAGE_FILE_HEADER pFileHeaderdll = &pNtHeaderdll->FileHeader;
	PIMAGE_OPTIONAL_HEADER pOptionalHeaderdll = &pNtHeaderdll->OptionalHeader;
	PIMAGE_SECTION_HEADER pSectionHeaderdll = IMAGE_FIRST_SECTION(pNtHeaderdll);

	//��ȡdll�ļ�������section�Ĵ�С
	DWORD imageSize = pOptionalHeaderdll->SizeOfImage;
	//DWORD imageBase = pOptionalHeaderdll->ImageBase;
	DWORD firstSectionAddr = pSectionHeaderdll->VirtualAddress;
	DWORD allSectionSize = imageSize - firstSectionAddr;

	// ������section�����ƽ�ȥ
	char* sectionBuff = (char*)(pSectionHeaderdll->VirtualAddress + (DWORD)hModule);
	BOOL bInsert = peUtil.InsertSection("51hook", allSectionSize, sectionBuff, 0xE00000E0);

	peUtil.RepairReloc((DWORD)hModule);
	DWORD OEPRVA = pPackInfo->newOEP - (DWORD)hModule;
	DWORD offset = OEPRVA - pSectionHeaderdll->VirtualAddress;
	BOOL bSet = peUtil.SetOEP(offset);
	peUtil.SaveFile("customPackReloc.exe");
	return 0;
}

