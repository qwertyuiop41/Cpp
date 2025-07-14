#pragma once
#include <Windows.h>
#include <iostream>

class CPeUtil
{
private:
	char* buffer;
	DWORD fileSize;
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNtHeaders;
	PIMAGE_OPTIONAL_HEADER pOptionalHeader;
	PIMAGE_FILE_HEADER pFileHeader;
	DWORD RvaToFoa(DWORD rva);
	
public:
	CPeUtil();
	~CPeUtil();
	BOOL LoadFile(const char* path);
	BOOL InsertSection(const char* sectionName, DWORD codeSize,  char* codebuff, DWORD dwCharacteristic);
	BOOL InitFileInfo();
	DWORD GetAlignmentSize(DWORD realSize,DWORD alignmentSize);
};

