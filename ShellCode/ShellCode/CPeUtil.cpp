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
	
	//��ö�����PE�ļ���С
	DWORD newFileSize = GetAlignmentSize(fileSize + codeSize,pOptionalHeader->FileAlignment);
	//�����µĻ��������PE�ļ�
	char* newbuffer = new char[newFileSize];
	memcpy_s(newbuffer, newFileSize, buffer, fileSize);
	fileSize = newFileSize;
	delete[] buffer;
	buffer = newbuffer;
	InitFileInfo();
	//�����������������ͷ
	PIMAGE_SECTION_HEADER pNewSectionHeader = GetLastSection();
	PIMAGE_SECTION_HEADER pLastSection = pNewSectionHeader - 1;
	//��������ͷ��������
	//�����ڴ��С
	pNewSectionHeader->Misc.VirtualSize = GetAlignmentSize(codeSize, pOptionalHeader->SectionAlignment);
	//�����ļ���С
	pNewSectionHeader->SizeOfRawData = GetAlignmentSize(codeSize, pOptionalHeader->FileAlignment);
	//������������
	strcpy_s((char*)pNewSectionHeader->Name, 8, sectionName);
	//����VirtualAddress
	pNewSectionHeader->VirtualAddress = pLastSection->VirtualAddress + GetAlignmentSize(pLastSection->Misc.VirtualSize,pOptionalHeader->SectionAlignment);
	//����PointerToRawData
	//��ΪSizeOfRawData���������ļ��ж�����С�����Բ���ҪGetAlignmentSize
	pNewSectionHeader->PointerToRawData = pLastSection->PointerToRawData + pLastSection->SizeOfRawData;
	//������������
	pNewSectionHeader->Characteristics = dwCharacteristic;

	//�޸�numberOfSection
	pFileHeader->NumberOfSections++;
	//�޸�sizeofimage
	pOptionalHeader->SizeOfImage += pNewSectionHeader->Misc.VirtualSize;

	//���Ǵ���ŵ��µ�����
	







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

	return pSectionHeader+(pFileHeader->NumberOfSections);
}
