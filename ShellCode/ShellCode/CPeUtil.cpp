#include "CPeUtil.h"

DWORD CPeUtil::RvaToFoa(DWORD rva)
{
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)
	{
		// 首先判断rva是否在当前区段的范围内
		if (rva > pSectionHeader->VirtualAddress && rva < pSectionHeader->VirtualAddress + pSectionHeader->Misc.VirtualSize)
		{
			//数据地址FOA=区段首地址FOA+数据地址RVA-区段首地址RVA
			return pSectionHeader->PointerToRawData + (rva - pSectionHeader->VirtualAddress);
		}
		pSectionHeader++;

	}
	return 0;
}

CPeUtil::CPeUtil()
{
	buffer = NULL;
	fileSize = 0;
	pDosHeader = NULL;
	pNtHeaders = NULL;
	pFileHeader = NULL;
	pOptionalHeader = NULL;
}

CPeUtil::~CPeUtil()
{
	if (buffer)
	{
		delete[] buffer;
		buffer = NULL;
	}
}

BOOL CPeUtil::LoadFile(const char* path)
{
	HANDLE hfile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == NULL)
	{
		MessageBoxA(NULL, "打开文件失败", "Error", MB_OK);
		return FALSE;
	}
	fileSize = GetFileSize(hfile, NULL);
	buffer = new char[fileSize];
	DWORD realSize = 0;
	BOOL bSuccess = ReadFile(hfile, buffer, fileSize, &realSize, NULL);
	if (!bSuccess)
	{
		MessageBoxA(NULL, "打开文件失败", "Error", MB_OK);
		CloseHandle(hfile);
		return FALSE;
	}
	MessageBoxA(NULL, "打开exe文件成功", "提示", MB_OK);
	InitFileInfo();
	return FALSE;
}

BOOL CPeUtil::InsertSection(const char* sectionName, DWORD codeSize, char* codebuff, DWORD dwCharacteristic)
{
	//1. 判断是否足够存放两个头的大小
	// SizeOfHeaders+DosHeader=区段头结尾的位置  不对x 因为SizeOfHeaders是fileAlignment对齐后的大小
	// first section header+number of sections*section header size
	// 循环section获得第一个区段头的位置
	//查看中间是否能容纳2*40

	// 计算节表（section table）的起始地址
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	DWORD firstSectionOffset = 0xffffffff;
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)
	{
		// 跳过虚拟区段（PointerToRawData为0）
		if (pSectionHeader->PointerToRawData != 0) {
			if (pSectionHeader->PointerToRawData < firstSectionOffset)
			{
				firstSectionOffset = pSectionHeader->PointerToRawData;
			}
		}
		pSectionHeader++;
	}
	DWORD sectionHeaderOffset =(DWORD) IMAGE_FIRST_SECTION(pNtHeaders)-(DWORD)pDosHeader;
	printf("sectionHeaderOffset: %x\n", sectionHeaderOffset);

	DWORD endOfSectionTableOffset =sectionHeaderOffset + pFileHeader->NumberOfSections * sizeof(IMAGE_SECTION_HEADER);

	printf("endOfSectionTableOffset: %x\n", endOfSectionTableOffset);
	printf("firstSectionOffset: %x\n", firstSectionOffset );
	printf("offset between end of section table and first section: %x\n", firstSectionOffset - endOfSectionTableOffset);

	if (firstSectionOffset- endOfSectionTableOffset <2* sizeof(IMAGE_SECTION_HEADER))
	{
		MessageBoxA(NULL, "计算节表末尾到第一个节之间的空隙不足以插入两个区段头", "Error", MB_OK);
		return FALSE;
	} 

	//---------------------------------------------------
	
	//获得对齐后的PE文件大小
	DWORD newFileSize = GetAlignmentSize(fileSize + codeSize,pOptionalHeader->FileAlignment);
	//创建新的缓冲区存放PE文件
	char* newbuffer = new char[newFileSize];
	memcpy_s(newbuffer, newFileSize, buffer, fileSize);
	fileSize = newFileSize;
	delete[] buffer;
	buffer = newbuffer;
	InitFileInfo();
	//给新增区段添加区段头
	PIMAGE_SECTION_HEADER pLastSection = GetLastSection();
	PIMAGE_SECTION_HEADER pNewSectionHeader = pLastSection +1;
	
	//给新区段头设置属性
	//设置内存大小
	pNewSectionHeader->Misc.VirtualSize = GetAlignmentSize(codeSize, pOptionalHeader->SectionAlignment);
	//设置文件大小
	pNewSectionHeader->SizeOfRawData = GetAlignmentSize(codeSize, pOptionalHeader->FileAlignment);
	//设置区段名称
	strcpy_s((char*)pNewSectionHeader->Name, 8, sectionName);
	//设置VirtualAddress
	pNewSectionHeader->VirtualAddress = pLastSection->VirtualAddress + GetAlignmentSize(pLastSection->Misc.VirtualSize,pOptionalHeader->SectionAlignment);
	//设置PointerToRawData
	pNewSectionHeader->PointerToRawData = pLastSection->PointerToRawData + GetAlignmentSize(pLastSection->SizeOfRawData,pOptionalHeader->FileAlignment);
	//设置区段属性
	pNewSectionHeader->Characteristics = dwCharacteristic;

	//修改numberOfSection
	pFileHeader->NumberOfSections++;
	//修改sizeofimage
	pOptionalHeader->SizeOfImage += pNewSectionHeader->Misc.VirtualSize;

	//将壳代码放到新的区段
	char* fileSectionAddr = GetLastSection()->PointerToRawData + buffer;
	memcpy(fileSectionAddr, codebuff, codeSize);
	MessageBoxA(NULL, "注入dll文件到新section成功", "提示", MB_OK);

	return TRUE;
	
}





BOOL CPeUtil::InitFileInfo()
{
	pDosHeader = (PIMAGE_DOS_HEADER)buffer;
	pNtHeaders = (PIMAGE_NT_HEADERS)(buffer + pDosHeader->e_lfanew);
	pFileHeader = &pNtHeaders->FileHeader;
	pOptionalHeader = &pNtHeaders->OptionalHeader;
	return TRUE;
}

DWORD CPeUtil::GetAlignmentSize(DWORD realSize, DWORD alignmentSize)
{
	if (realSize%alignmentSize==0)
	{
		return realSize;
	}
	return (realSize/alignmentSize+1)*alignmentSize;
}

PIMAGE_SECTION_HEADER CPeUtil::GetLastSection()
{
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);

	return pSectionHeader+(pFileHeader->NumberOfSections-1);
}

BOOL CPeUtil::SaveFile(const char* path)
{
	HANDLE hfile = CreateFileA(path, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD realSize = 0;
	BOOL bSuccess = WriteFile(hfile, buffer, fileSize, &realSize, NULL);
	CloseHandle(hfile);  
	return 0;
}

//初始化.text区段（通常是第一区段）
BOOL CPeUtil::EncodeSections()
{
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	DWORD key = 0x51;
	char* pData = buffer + pSectionHeader->PointerToRawData;
	for (DWORD i = 0; i < pSectionHeader->SizeOfRawData; i++)
	{
		pData[i] ^= key;
	}

	return TRUE;
}

DWORD CPeUtil::GetOldOEP()
{
	DWORD oldOEP = pOptionalHeader->AddressOfEntryPoint + pOptionalHeader->ImageBase;

	return oldOEP;
}

BOOL CPeUtil::SetOEP(DWORD newOEPRVA)
{
	PIMAGE_SECTION_HEADER lastSection = GetLastSection();
	pOptionalHeader->AddressOfEntryPoint = lastSection->VirtualAddress + newOEPRVA;
	return TRUE;
}

BOOL CPeUtil::RepairReloc(DWORD imageBase)
{
	PIMAGE_DOS_HEADER pDosHeaderDll = (PIMAGE_DOS_HEADER)imageBase;
	PIMAGE_NT_HEADERS pNtHeaderDll = (PIMAGE_NT_HEADERS)(pDosHeaderDll->e_lfanew + imageBase);
	PIMAGE_OPTIONAL_HEADER pOptionalHeaderDll = (PIMAGE_OPTIONAL_HEADER)(&pNtHeaderDll->OptionalHeader);
	IMAGE_DATA_DIRECTORY dataDirectoryDll = (IMAGE_DATA_DIRECTORY)(pOptionalHeaderDll->DataDirectory[5]);
	PIMAGE_BASE_RELOCATION pBaseRelocationDll = (PIMAGE_BASE_RELOCATION)(dataDirectoryDll.VirtualAddress + imageBase);
	PIMAGE_SECTION_HEADER pSectionHeaderDll = IMAGE_FIRST_SECTION(pNtHeaderDll);
	while (pBaseRelocationDll->SizeOfBlock!=0)
	{
		char* start = (char*)pBaseRelocationDll;
		DWORD num = (pBaseRelocationDll->SizeOfBlock - 8) / 2;
		start += 8;
		for (int i = 0; i < num; i++){
			WORD* offset = (WORD*)start;
			if ((0x3000 & *offset) == 0x3000) {
				//VirtualAdress+Word数据低12位=真正的RVA
				WORD offset2= *offset & 0x0FFF;
				WORD relocRva = (offset2 + pBaseRelocationDll->VirtualAddress);
				DWORD* relocAddr = (DWORD*)(relocRva+imageBase);
				PIMAGE_SECTION_HEADER lastSection = GetLastSection();
				DWORD newFileSection = (DWORD)(lastSection->PointerToRawData + buffer);
				DWORD newSectionAddr = (DWORD)(lastSection->VirtualAddress + pOptionalHeader->ImageBase);

				// 通过在区段中偏移地址相同计算destAddr
				DWORD destAddr = newFileSection + (DWORD)relocAddr - (DWORD)(pSectionHeaderDll->VirtualAddress+imageBase);
				// destAddr是要修改的地址在新文件中的位置
				// 然后对destAddr地址中存储的值进行 relocation 修正，由注入进来的file对齐，变成运行过程的内存对齐
				*(DWORD*)destAddr = newSectionAddr + (*(DWORD*)destAddr - imageBase) - pSectionHeaderDll->VirtualAddress;
			}
			start += 2;
		}
		pBaseRelocationDll = (PIMAGE_BASE_RELOCATION)((DWORD)pBaseRelocationDll + pBaseRelocationDll->SizeOfBlock);
	}


	return 0;
}


