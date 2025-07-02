#pragma once
#include<Windows.h>
#include<iostream>
class PeUtil
{
public:
	PeUtil();
	~PeUtil();
	BOOL LoadFile(const char*path);
	BOOL InitPeInfo();
	void PrintSectionHeaders();
	void GetExportTable();
	void GetImportTable();
private:
	char* buffer;
	DWORD fileSize;
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNtHeaders;
	PIMAGE_FILE_HEADER pFileHeader;
	PIMAGE_OPTIONAL_HEADER pOptionalHeader;
	DWORD RvaToFoa(DWORD rva);
};

