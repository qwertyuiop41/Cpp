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
	//e_magic��PE�ֶεı�ʾ0x5A4B��MZ�����������0x5A4B����Ϊ������ЧPE�ļ�
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return FALSE;
	}

	pNtHeaders = (PIMAGE_NT_HEADERS)(buffer + pDosHeader->e_lfanew);
	// ���PEͷ��ǩ��(0x4550)
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
		//pSectionHeader->Name����\0��β���ַ��������ǹ̶���С��ԭʼ�ַ����飬���Բ���ֱ��%s
		//Ϊ���ܴ�ӡpSectionHeader->Name������pSectionHeader->Name��name�У��ұ������һ���ֽ�\0��
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
		//��ҪAddressOfFunctionsÿһ���Ӧ��ֵ
		printf("Function Address: %x\n", *funaddr);
		// AddressOfNames �� AddressOfNameOrdinals �ĳ�����һ���ģ����� NumberOfNames��
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
		// �����ж�rva�Ƿ��ڵ�ǰ���εķ�Χ��
		if (rva>pSectionHeader->VirtualAddress&&rva< pSectionHeader->VirtualAddress+pSectionHeader->Misc.VirtualSize)
		{
			//���ݵ�ַFOA=�����׵�ַFOV+���ݵ�ַRVA-�����׵�ַRVA
			return pSectionHeader->PointerToRawData + (rva - pSectionHeader->VirtualAddress);
		}
		pSectionHeader++;

	}
	return 0;
}
