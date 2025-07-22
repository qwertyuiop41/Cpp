#include "CPeUtil.h"

DWORD CPeUtil::RvaToFoa(DWORD rva)
{
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)
	{
		// �����ж�rva�Ƿ��ڵ�ǰ���εķ�Χ��
		if (rva > pSectionHeader->VirtualAddress && rva < pSectionHeader->VirtualAddress + pSectionHeader->Misc.VirtualSize)
		{
			//���ݵ�ַFOA=�����׵�ַFOA+���ݵ�ַRVA-�����׵�ַRVA
			return pSectionHeader->PointerToRawData + (rva - pSectionHeader->VirtualAddress);
		}
		pSectionHeader++;

	}
	return 0;
}

DWORD CPeUtil::FoaToRva(DWORD foa)
{
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)
	{
		// �����ж�rva�Ƿ��ڵ�ǰ���εķ�Χ��
		if (foa > pSectionHeader->PointerToRawData && foa < pSectionHeader->PointerToRawData + pSectionHeader->SizeOfRawData)
		{
			//���ݵ�ַFOA=�����׵�ַFOA+���ݵ�ַRVA-�����׵�ַRVA
			return pSectionHeader->VirtualAddress + (foa - pSectionHeader->PointerToRawData);
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
	importTableSize = 0;
	pNewImportDescriptor = NULL;
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
		MessageBoxA(NULL, "���ļ�ʧ��", "Error", MB_OK);
		return FALSE;
	}
	fileSize = GetFileSize(hfile, NULL);
	buffer = new char[fileSize];
	DWORD realSize = 0;
	BOOL bSuccess = ReadFile(hfile, buffer, fileSize, &realSize, NULL);
	if (!bSuccess)
	{
		MessageBoxA(NULL, "���ļ�ʧ��", "Error", MB_OK);
		CloseHandle(hfile);
		return FALSE;
	}
	MessageBoxA(NULL, "��exe�ļ��ɹ�", "��ʾ", MB_OK);
	InitFileInfo();
	return FALSE;
}

BOOL CPeUtil::InsertSection(const char* sectionName, DWORD codeSize, char* codebuff, DWORD dwCharacteristic)
{
	//1. �ж��Ƿ��㹻�������ͷ�Ĵ�С
	// SizeOfHeaders+DosHeader=����ͷ��β��λ��  ����x ��ΪSizeOfHeaders��fileAlignment�����Ĵ�С
	// first section header+number of sections*section header size
	// ѭ��section��õ�һ������ͷ��λ��
	//�鿴�м��Ƿ�������2*40

	// ����ڱ�section table������ʼ��ַ
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);
	DWORD firstSectionOffset = 0xffffffff;
	for (int i = 0; i < pFileHeader->NumberOfSections; i++)
	{
		// �����������Σ�PointerToRawDataΪ0��
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
		MessageBoxA(NULL, "����ڱ�ĩβ����һ����֮��Ŀ�϶�����Բ�����������ͷ", "Error", MB_OK);
		return FALSE;
	} 

	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	
	//��ö�����PE�ļ���С
	DWORD originalCodeSize = codeSize;
	codeSize += importTableSize;
	DWORD newFileSize = GetAlignmentSize(fileSize + codeSize,pOptionalHeader->FileAlignment);
	//�����µĻ��������PE�ļ�
	char* newbuffer = new char[newFileSize] {};
	memcpy_s(newbuffer, newFileSize, buffer, fileSize);
	fileSize = newFileSize;
	delete[] buffer;
	buffer = newbuffer;
	InitFileInfo();
	//�����������������ͷ
	PIMAGE_SECTION_HEADER pLastSection = GetLastSection();
	//PIMAGE_SECTION_HEADER pNewSectionHeader = pLastSection +1;
	pLastSection++;
	
	//��������ͷ��������
	//�����ڴ��С
	pLastSection->Misc.VirtualSize = GetAlignmentSize(codeSize, pOptionalHeader->SectionAlignment);
	//�����ļ���С
	pLastSection->SizeOfRawData = GetAlignmentSize(codeSize, pOptionalHeader->FileAlignment);
	//������������
	strcpy_s((char*)pLastSection->Name, 8, sectionName);
	//����VirtualAddress
	PIMAGE_SECTION_HEADER pLastSection2 = GetLastSection();
	pLastSection->VirtualAddress = pLastSection2->VirtualAddress + GetAlignmentSize(pLastSection2->Misc.VirtualSize,pOptionalHeader->SectionAlignment);
	//����PointerToRawData
	
	pLastSection->PointerToRawData = pLastSection2->PointerToRawData + GetAlignmentSize(pLastSection2->SizeOfRawData,pOptionalHeader->FileAlignment);
	//������������
	pLastSection->Characteristics = dwCharacteristic;

	//�޸�numberOfSection
	pFileHeader->NumberOfSections++;
	//�޸�sizeofimage
	pOptionalHeader->SizeOfImage += GetAlignmentSize(codeSize, pOptionalHeader->SectionAlignment);

	//���Ǵ���ŵ��µ�����
	char* fileSectionAddr = pLastSection->PointerToRawData + buffer;
	memcpy(fileSectionAddr, codebuff, codeSize);

	//���µ����ŵ��µ�����
	fileSectionAddr += originalCodeSize;
	memcpy(fileSectionAddr, pNewImportDescriptor, importTableSize);

	//�����λ��ָ��������
	PIMAGE_DATA_DIRECTORY pImportDirectory = &pOptionalHeader->DataDirectory[1];
	pImportDirectory->VirtualAddress = FoaToRva((DWORD)(fileSectionAddr-buffer));


	MessageBoxA(NULL, "ע��dll�ļ�����section�ɹ�", "��ʾ", MB_OK);

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

//��ʼ��.text���Σ�ͨ���ǵ�һ���Σ�
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
	while (pBaseRelocationDll->SizeOfBlock>=8)
	{
		char* start = (char*)pBaseRelocationDll;
		DWORD num = (pBaseRelocationDll->SizeOfBlock - 8) / 2;
		start += 8;
		for (int i = 0; i < num; i++){
			WORD* offset = (WORD*)start;
			if ((0x3000 & *offset) == 0x3000) {
				//VirtualAdress+Word���ݵ�12λ=������RVA
				WORD relocRva = (*offset & 0x0FFF + pBaseRelocationDll->VirtualAddress);
				DWORD* relocAddr = (DWORD*)(relocRva+imageBase);
				PIMAGE_SECTION_HEADER lastSection = GetLastSection();
				DWORD newFileSection = (DWORD)(lastSection->PointerToRawData + buffer);
				DWORD newSectionAddr = (DWORD)(lastSection->VirtualAddress + pOptionalHeader->ImageBase);

				// ͨ����������ƫ�Ƶ�ַ��ͬ����destAddr
				DWORD destAddr = newFileSection + (DWORD)relocAddr - (DWORD)(pSectionHeaderDll->VirtualAddress+imageBase);
				// destAddr��Ҫ�޸ĵĵ�ַ�����ļ��е�λ��
				// Ȼ���destAddr��ַ�д洢��ֵ���� relocation ��������ע�������file���룬������й��̵��ڴ����
				*(DWORD*)destAddr = newSectionAddr + (*(DWORD*)destAddr - imageBase) - pSectionHeaderDll->VirtualAddress;
			}
			start += 2;
		}
		pBaseRelocationDll = (PIMAGE_BASE_RELOCATION)((DWORD)pBaseRelocationDll + pBaseRelocationDll->SizeOfBlock);
	}


	return 0;
}

BOOL CPeUtil::GetImportTable()
{
	IMAGE_DATA_DIRECTORY dictionary = pOptionalHeader->DataDirectory[1];
	PIMAGE_IMPORT_DESCRIPTOR pImportDictionary = (PIMAGE_IMPORT_DESCRIPTOR)(RvaToFoa(dictionary.VirtualAddress) + buffer);
	PIMAGE_IMPORT_DESCRIPTOR pFirstDictionary = (PIMAGE_IMPORT_DESCRIPTOR)(RvaToFoa(dictionary.VirtualAddress) + buffer);

	DWORD tableLen = 0;

	while (pImportDictionary->Name!=NULL)
	{
		tableLen++;
		pImportDictionary++;
	}
	// ����һ��λ����������0
	tableLen++;
	importTableSize = tableLen * sizeof(IMAGE_IMPORT_DESCRIPTOR);
	pNewImportDescriptor = new IMAGE_IMPORT_DESCRIPTOR[tableLen]{};
	memcpy(pNewImportDescriptor, pFirstDictionary, (tableLen - 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR));
	return TRUE;

}


