#include<Windows.h>
#include<iostream>
#include "CPeUtil.h"


//为什么还要定义一遍？
//不定义的话，无法使用PACKINFO的信息
//编译器会报错：未定义标识符 PACKINFO？？？
typedef struct _PACKINFO {
	DWORD newOEP;
	DWORD oldOEP;
}PACKINFO, * PPACKINFO;


int main() {
	CPeUtil peUtil;
	BOOL bSuccess = peUtil.LoadFile("FateMouse_upx.exe");

	peUtil.EncodeSections();
	HMODULE hModule = LoadLibraryA("pack.dll");
	// GetProcAddress 的语义是返回符号地址
	PPACKINFO pPackInfo = (PPACKINFO)GetProcAddress(hModule, "g_PackInfo");
	pPackInfo->oldOEP = peUtil.GetOldOEP();
	//peUtil.GetImportTable();
	
	PIMAGE_DOS_HEADER pDosHeaderdll = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNtHeaderdll = (PIMAGE_NT_HEADERS)((DWORD)hModule + pDosHeaderdll->e_lfanew);
	PIMAGE_FILE_HEADER pFileHeaderdll = &pNtHeaderdll->FileHeader;
	PIMAGE_OPTIONAL_HEADER pOptionalHeaderdll = &pNtHeaderdll->OptionalHeader;
	PIMAGE_SECTION_HEADER pSectionHeaderdll = IMAGE_FIRST_SECTION(pNtHeaderdll);

	//获取dll文件的所有section的大小
	DWORD imageSize = pOptionalHeaderdll->SizeOfImage;
	//DWORD imageBase = pOptionalHeaderdll->ImageBase;
	DWORD firstSectionAddr = pSectionHeaderdll->VirtualAddress;
	DWORD allSectionSize = imageSize - firstSectionAddr;

	// 把所有section都复制进去
	char* sectionBuff = (char*)(pSectionHeaderdll->VirtualAddress + (DWORD)hModule);
	BOOL bInsert = peUtil.InsertSection("51hook", allSectionSize, sectionBuff, 0xE00000E0);

	peUtil.RepairReloc((DWORD)hModule);
	DWORD OEPRVA = pPackInfo->newOEP - (DWORD)hModule;
	DWORD offset = OEPRVA - pSectionHeaderdll->VirtualAddress;
	BOOL bSet = peUtil.SetOEP(offset);
	peUtil.SaveFile("customPackReloc.exe");
	return 0;
}

