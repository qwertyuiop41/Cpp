#include<Windows.h>
#include <iostream>
#include "pack.h"
//将代码段数据段合并在一起
#pragma comment(linker, "/merge:.data=.text")
#pragma comment(linker, "/merge:.rdata=.text")
//因为数据段是可写的，所以我们在.text段中也添加可写属性
//这一步设置的是 PE 文件中节头里的 Characteristics 字段，
//这只在加载器（如 Windows 的 PE 加载器）加载文件时，会将节的初始内存页权限设置为指定值。
//但是！Windows 的加载器不总是完全遵循这些节属性，
//在 Windows 10/11 上，PE 加载器会默认让 .text 保持只读 + 可执行（RX），因为代码段不应写入。
//所以，需要在运行时再次手动修改内存保护（才是生效的）
#pragma comment(linker, "/section:.text,RWE")

//壳代码有两个作用：解密+保护


//动态获取函数地址（动态链接）
//定义函数指针类型 MyLoadLibraryExA，可以将动态获取的地址转换为可调用的函数
typedef HMODULE (WINAPI* MyLoadLibraryExA)(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD  dwFlags
);

typedef FARPROC (WINAPI* MyGetProcAddress)(
	HMODULE hModule,
	LPCSTR  lpProcName
);

typedef HMODULE (WINAPI* MyGetModuleHandleA)(
	LPCSTR lpModuleName
);

typedef BOOL (WINAPI* MyVirtualProtect)(
	LPVOID lpAddress,
	SIZE_T dwSize,
	DWORD flNewProtect,
	PDWORD lpflOldProtect
);

typedef int (WINAPI* MyMessageBoxA)(
	HWND   hWnd,
	LPCSTR lpText,
	LPCSTR lpCaption,
	UINT   uType
);

MyLoadLibraryExA g_MyLoadLibraryExA = NULL;
MyGetProcAddress g_MyGetProcAddress = NULL;
MyGetModuleHandleA g_MyGetModuleHandleA = NULL;
MyVirtualProtect g_MyVirtualProtect = NULL;
MyMessageBoxA g_MyMessageBoxA = NULL;




PACKINFO g_PackInfo = { (DWORD)packStart};

DWORD GetKernel() {
	DWORD base = 0;
	_asm {
		mov eax, dword ptr fs : [0x30] //获取PEB地址
		mov eax, [eax + 0xc] //获取LDR结构体地址
		mov eax, [eax + 0x1c]//list
		mov eax, [eax]//list第二项 kernel32.dll
		mov eax, [eax + 0x8]//kernel32.dll基址
		mov base, eax //将基址存储到base变量中
	}
	return base;
}

//内存中加载的位置
DWORD MyGetProcessAddress(DWORD hModule, LPCSTR lpProcName) {
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(hModule + pDosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNtHeaders->OptionalHeader;
	IMAGE_DATA_DIRECTORY dictionary = pOptionalHeader->DataDirectory[0];
	//直接使用RVA，不需要转化了 
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(dictionary.VirtualAddress + hModule);
	DWORD* funaddr = (DWORD*)((pExportDirectory->AddressOfFunctions) + hModule);
	DWORD* funname = (DWORD*)((pExportDirectory->AddressOfNames) + hModule);
	WORD* funord = (WORD*)((pExportDirectory->AddressOfNameOrdinals) + hModule);
	for (int i = 0; i < pExportDirectory->NumberOfNames; i++)
	{
		char* name = (char*)(funname[i] + hModule);
		if (strcmp(name,lpProcName)==0)
		{
			return funaddr[funord[i]] + hModule; //返回函数地址
		}
	}
	return 0; //没有找到函数
}

//通过给自定义的函数指针赋值获得相应的函数
void GetFunctions() {
	//1. 获取kernel32或者kernelbase
	DWORD base = GetKernel();
	//2. 获取LoadLibraryExA，因为win7里kernelbase里没有LoadLibrary
	g_MyLoadLibraryExA = (MyLoadLibraryExA)MyGetProcessAddress(base, "LoadLibraryExA");
	//获取kernel32
	HMODULE kernel32 = g_MyLoadLibraryExA("kernel32.dll", 0, 0);
	// MyGetProcAddress是要遍历导出表一一匹配，消耗时间，windows的GetProcAddress可以直接获得函数地址
	g_MyGetProcAddress = (MyGetProcAddress)MyGetProcessAddress((DWORD)kernel32, "GetProcAddress");
	g_MyGetModuleHandleA = (MyGetModuleHandleA)g_MyGetProcAddress(kernel32, "GetModuleHandleA");
	g_MyVirtualProtect = (MyVirtualProtect)g_MyGetProcAddress(kernel32, "VirtualProtect");
	HMODULE user32 = g_MyLoadLibraryExA("user32.dll", 0, 0);
	g_MyMessageBoxA = (MyMessageBoxA)g_MyGetProcAddress(user32, "MessageBoxA");
	
}


//解码代码段
BOOL DecodeSections() {
	int key = 0x51;
	//1. 获取模块基址-》GetModuleHandleA，传0代表获得当前pe的模块基址
	HMODULE hModule = g_MyGetModuleHandleA(0);
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((DWORD)hModule + pDosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNtHeaders->OptionalHeader;
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	char* pData = (char*)((DWORD)hModule + pSectionHeader->VirtualAddress);

	//运行时修改第一个section(.text)属性为可写
	DWORD oldProtect = 0;
	g_MyVirtualProtect(pData, pSectionHeader->SizeOfRawData, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (DWORD i = 0; i < pSectionHeader->SizeOfRawData; i++)
	{
		pData[i] ^= key;
	}
	g_MyVirtualProtect(pData, pSectionHeader->SizeOfRawData, oldProtect, &oldProtect);
	return TRUE;
}



//加壳main
__declspec(naked) void packStart() {
	_asm pushad
	GetFunctions();
	DecodeSections();
	g_MyMessageBoxA(
		0,
		"壳代码执行成功",
		"Pack Test",
		MB_OK
		);
	_asm popad
	_asm jmp g_PackInfo.oldOEP
}

