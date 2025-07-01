#include "PeUtil.h"

PeUtil::PeUtil()
{
	buffer=NULL;
	fileSize=0;
	pDosHeader=NULL;
	pNtHeaders = NULL;
	pFileHeader = NULL;
	pOptionalHeader = NULL;
}

PeUtil::~PeUtil()
{
	if (buffer)
	{
		delete[] buffer;
		buffer = NULL;
	}
}

BOOL PeUtil::LoadFile(const char* path)
{
	HANDLE hfile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile==NULL)
	{
		return FALSE;
	}
	fileSize = GetFileSize(hfile, NULL);
	buffer = new char[fileSize];
	DWORD realSize = 0;
	BOOL bSuccess = ReadFile(hfile, buffer, fileSize, &realSize, NULL);
	if (!bSuccess)
	{
		CloseHandle(hfile);
		return FALSE;
	}
	if (InitPeInfo())
	{
		CloseHandle(hfile);
		return TRUE;
	}
	CloseHandle(hfile);
	return FALSE;
}

BOOL PeUtil::InitPeInfo()
{
	pDosHeader = (PIMAGE_DOS_HEADER)buffer;
	//e_magic是PE字段的表示0x5A4B（MZ），如果不是0x5A4B则认为不是有效PE文件
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return FALSE;
	}

	pNtHeaders = (PIMAGE_NT_HEADERS)(buffer + pDosHeader->e_lfanew);
	// 检查PE头的签名(0x4550)
	DWORD Signature = pNtHeaders->Signature;
	if (Signature != IMAGE_NT_SIGNATURE)
	{
		return FALSE;
	}
	pFileHeader = &pNtHeaders->FileHeader;
	pOptionalHeader = &pNtHeaders->OptionalHeader;
	return TRUE;
}

void PeUtil::PrintSectionHeaders()
{
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)
	{
		//pSectionHeader->Name不是\0结尾的字符串，而是固定大小的原始字符数组，所以不能直接%s
		//为了能打印pSectionHeader->Name，拷贝pSectionHeader->Name到name中，且保留最后一个字节\0；
		char name[9] = { 0 };
		memcpy_s(name,9, pSectionHeader->Name,8);
		printf("Section Name %s:\n", name);
		pSectionHeader++;

	}
}

void PeUtil::GetExportTable()
{
	IMAGE_DATA_DIRECTORY dictionary = pOptionalHeader->DataDirectory[0];
	PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(RvaToFoa(dictionary.VirtualAddress)+buffer);
	char* dllName = (char*)(RvaToFoa(pExportDirectory->Name) + buffer);
	printf("DLL Name: %s\n", dllName);
	DWORD* funaddr = (DWORD*)(RvaToFoa(pExportDirectory->AddressOfFunctions) + buffer);
	DWORD* funname = (DWORD*)(RvaToFoa(pExportDirectory->AddressOfNames) + buffer);
	WORD* funord = (WORD*)(RvaToFoa(pExportDirectory->AddressOfNameOrdinals) + buffer);
	for (int i = 0; i < pExportDirectory->NumberOfFunctions; i++)
	{
		//是要AddressOfFunctions每一项对应的值
		printf("Function Address: %x\n", *funaddr);
		// AddressOfNames 和 AddressOfNameOrdinals 的长度是一样的，都是 NumberOfNames。
		for (int j = 0; j < pExportDirectory->NumberOfNames; j++)
		{
			if (funord[j] == i) {
				char* name = RvaToFoa(funname[j])+buffer;
				printf("Function Name: %s\n", name);
				break;
			}
		}
		funaddr++;


	}
}

DWORD PeUtil::RvaToFoa(DWORD rva)
{
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)
	{
		// 首先判断rva是否在当前区段的范围内
		if (rva>pSectionHeader->VirtualAddress&&rva< pSectionHeader->VirtualAddress+pSectionHeader->Misc.VirtualSize)
		{
			//数据地址FOA=区段首地址FOV+数据地址RVA-区段首地址RVA
			return pSectionHeader->PointerToRawData + (rva - pSectionHeader->VirtualAddress);
		}
		pSectionHeader++;

	}
	return 0;
}
