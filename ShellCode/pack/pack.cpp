#include<Windows.h>
#include <iostream>
#include "pack.h"
//����������ݶκϲ���һ��
#pragma comment(linker, "/merge:.data=.text")
#pragma comment(linker, "/merge:.rdata=.text")
//��Ϊ���ݶ��ǿ�д�ģ�����������.text����Ҳ��ӿ�д����
//��һ�����õ��� PE �ļ��н�ͷ��� Characteristics �ֶΣ�
//��ֻ�ڼ��������� Windows �� PE �������������ļ�ʱ���Ὣ�ڵĳ�ʼ�ڴ�ҳȨ������Ϊָ��ֵ��
//���ǣ�Windows �ļ�������������ȫ��ѭ��Щ�����ԣ�
//�� Windows 10/11 �ϣ�PE ��������Ĭ���� .text ����ֻ�� + ��ִ�У�RX������Ϊ����β�Ӧд�롣
//���ԣ���Ҫ������ʱ�ٴ��ֶ��޸��ڴ汣����������Ч�ģ�
#pragma comment(linker, "/section:.text,RWE")

//�Ǵ������������ã�����+����


//��̬��ȡ������ַ����̬���ӣ�
//���庯��ָ������ MyLoadLibraryExA�����Խ���̬��ȡ�ĵ�ַת��Ϊ�ɵ��õĺ���
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
		mov eax, dword ptr fs : [0x30] //��ȡPEB��ַ
		mov eax, [eax + 0xc] //��ȡLDR�ṹ���ַ
		mov eax, [eax + 0x1c]//list
		mov eax, [eax]//list�ڶ��� kernel32.dll
		mov eax, [eax + 0x8]//kernel32.dll��ַ
		mov base, eax //����ַ�洢��base������
	}
	return base;
}

//�ڴ��м��ص�λ��
DWORD MyGetProcessAddress(DWORD hModule, LPCSTR lpProcName) {
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(hModule + pDosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNtHeaders->OptionalHeader;
	IMAGE_DATA_DIRECTORY dictionary = pOptionalHeader->DataDirectory[0];
	//ֱ��ʹ��RVA������Ҫת���� 
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(dictionary.VirtualAddress + hModule);
	DWORD* funaddr = (DWORD*)((pExportDirectory->AddressOfFunctions) + hModule);
	DWORD* funname = (DWORD*)((pExportDirectory->AddressOfNames) + hModule);
	WORD* funord = (WORD*)((pExportDirectory->AddressOfNameOrdinals) + hModule);
	for (int i = 0; i < pExportDirectory->NumberOfNames; i++)
	{
		char* name = (char*)(funname[i] + hModule);
		if (strcmp(name,lpProcName)==0)
		{
			return funaddr[funord[i]] + hModule; //���غ�����ַ
		}
	}
	return 0; //û���ҵ�����
}

//ͨ�����Զ���ĺ���ָ�븳ֵ�����Ӧ�ĺ���
void GetFunctions() {
	//1. ��ȡkernel32����kernelbase
	DWORD base = GetKernel();
	//2. ��ȡLoadLibraryExA����Ϊwin7��kernelbase��û��LoadLibrary
	g_MyLoadLibraryExA = (MyLoadLibraryExA)MyGetProcessAddress(base, "LoadLibraryExA");
	//��ȡkernel32
	HMODULE kernel32 = g_MyLoadLibraryExA("kernel32.dll", 0, 0);
	// MyGetProcAddress��Ҫ����������һһƥ�䣬����ʱ�䣬windows��GetProcAddress����ֱ�ӻ�ú�����ַ
	g_MyGetProcAddress = (MyGetProcAddress)MyGetProcessAddress((DWORD)kernel32, "GetProcAddress");
	g_MyGetModuleHandleA = (MyGetModuleHandleA)g_MyGetProcAddress(kernel32, "GetModuleHandleA");
	g_MyVirtualProtect = (MyVirtualProtect)g_MyGetProcAddress(kernel32, "VirtualProtect");
	HMODULE user32 = g_MyLoadLibraryExA("user32.dll", 0, 0);
	g_MyMessageBoxA = (MyMessageBoxA)g_MyGetProcAddress(user32, "MessageBoxA");
	
}


//��������
BOOL DecodeSections() {
	int key = 0x51;
	//1. ��ȡģ���ַ-��GetModuleHandleA����0�����õ�ǰpe��ģ���ַ
	HMODULE hModule = g_MyGetModuleHandleA(0);
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((DWORD)hModule + pDosHeader->e_lfanew);
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNtHeaders->OptionalHeader;
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	char* pData = (char*)((DWORD)hModule + pSectionHeader->VirtualAddress);

	//����ʱ�޸ĵ�һ��section(.text)����Ϊ��д
	DWORD oldProtect = 0;
	g_MyVirtualProtect(pData, pSectionHeader->SizeOfRawData, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (DWORD i = 0; i < pSectionHeader->SizeOfRawData; i++)
	{
		pData[i] ^= key;
	}
	g_MyVirtualProtect(pData, pSectionHeader->SizeOfRawData, oldProtect, &oldProtect);
	return TRUE;
}



//�ӿ�main
__declspec(naked) void packStart() {
	_asm pushad
	GetFunctions();
	DecodeSections();
	g_MyMessageBoxA(
		0,
		"�Ǵ���ִ�гɹ�",
		"Pack Test",
		MB_OK
		);
	_asm popad
	_asm jmp g_PackInfo.oldOEP
}

