#include<Windows.h>
#include<iostream>

int main() {
	HANDLE hfile = CreateFileA("crackme1.exe", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD fileSize = GetFileSize(hfile, NULL);
	char* buffer = new char[fileSize];
	DWORD realSize = 0;
	BOOL bSuccess = ReadFile(hfile, buffer, fileSize, &realSize, NULL);
	PIMAGE_NT_HEADERS pNtHeaders = NULL;
	PIMAGE_OPTIONAL_HEADER pOptionalHeader = NULL;
	if(bSuccess ) {
		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)buffer;
		//e_magic是PE字段的表示0x5A4B（MZ），如果不是0x5A4B则认为不是有效PE文件
		if (pDosHeader->e_magic!=0x5A4D)
		{
			printf("This is not a valid PE file.\n");
			delete[] buffer;
			CloseHandle(hfile);
			return 0;

		}
		//64（DOS header）+ 128（stub）= 0xF8（248）
		printf("e_lfanew: %d\n", pDosHeader->e_lfanew);

		// 因为header部分不存在对齐填补（RAV和FOA是一样的），所以不需要进行转换
		pNtHeaders = (PIMAGE_NT_HEADERS)(buffer + pDosHeader->e_lfanew);
		// 检查PE头的签名(0x4550)
		DWORD Signature = pNtHeaders->Signature;
		if (Signature!=0x4550)
		{
			printf("This is not a valid PE file.\n");
			delete[] buffer;
			CloseHandle(hfile);
			return 0;
		}
		PIMAGE_FILE_HEADER pFileHeader = &pNtHeaders->FileHeader;
		printf("Machine: %x\n", pFileHeader->Machine);
		printf("NumberOfSections: %d\n", pFileHeader->NumberOfSections);
		printf("Size of OptionalHeader: %d\n", pFileHeader->SizeOfOptionalHeader);
		printf("Characteristics: %x\n", pFileHeader->Characteristics);


		pOptionalHeader = &pNtHeaders->OptionalHeader;
		//表示是32位PE文件还是64位PE文件 10B32位 20B64位
		printf("Magic: %x\n", pOptionalHeader->Magic);

		delete[] buffer;
		CloseHandle(hfile);
		return 1;


	}

	delete[] buffer;
	CloseHandle(hfile);
	return 0;



}