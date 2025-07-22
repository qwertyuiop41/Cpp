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
	
	PIMAGE_DOS_HEADER pDosHeaderdll = (PIMAGE_DOS_HEADER)hModule;
	//要把hModule改成(DWORD)hModule，不然会报错
	// 直接(HMODULE + DWORD) 会导致编译器认为你要以 sizeof(HMODULE) 为单位进行指针偏移，而不是字节偏移！
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

//注意：
//不能通过下面的方式获得entry point
//因为entry point指的是DLL的标准入口点（通常是 DllMain 或编译器生成的初始化代码）
//是错误的入口点！会执行DLL初始化代码而不是壳代码
//DWORD dllOEP = pNtHeaderdll->OptionalHeader.AddressOfEntryPoint;
//DWORD packStartOffset = dllOEP - pSectionHeaderdll->VirtualAddress;


//int main() {
//	CPeUtil peUtil;
//	BOOL bSuccess = peUtil.LoadFile("FateMouse_upx.exe");
//	HMODULE hModule = LoadLibraryA("pack.dll");
//	// GetProcAddress 的语义是返回符号地址
//	PPACKINFO pPackInfo = (PPACKINFO)GetProcAddress(hModule, "g_PackInfo");
//	pPackInfo->oldOEP = peUtil.GetOldOEP();
//	PIMAGE_DOS_HEADER pDosHeaderdll = (PIMAGE_DOS_HEADER)hModule;
//	//要把hModule改成(DWORD)hModule，不然会报错
//	// 直接(HMODULE + DWORD) 会导致编译器认为你要以 sizeof(HMODULE) 为单位进行指针偏移，而不是字节偏移！
//	PIMAGE_NT_HEADERS pNtHeaderdll = (PIMAGE_NT_HEADERS)((DWORD)hModule + pDosHeaderdll->e_lfanew);
//	PIMAGE_SECTION_HEADER pSectionHeaderdll = IMAGE_FIRST_SECTION(pNtHeaderdll);
//	char* sectionBuff = (char*)(pSectionHeaderdll->VirtualAddress + (DWORD)hModule);
//
//	BOOL bInsert = peUtil.InsertSection("51hook", pSectionHeaderdll->Misc.VirtualSize, sectionBuff, 0xE00000E0);
//
//	peUtil.SaveFile("customPack2.exe");
//	return 0;
//}