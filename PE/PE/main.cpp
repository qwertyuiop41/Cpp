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
		//e_magic��PE�ֶεı�ʾ0x5A4B��MZ�����������0x5A4B����Ϊ������ЧPE�ļ�
		if (pDosHeader->e_magic!=0x5A4D)
		{
			printf("This is not a valid PE file.\n");
			delete[] buffer;
			CloseHandle(hfile);
			return 0;

		}
		//64��DOS header��+ 128��stub��= 0xF8��248��
		printf("e_lfanew: %d\n", pDosHeader->e_lfanew);

		// ��Ϊheader���ֲ����ڶ������RAV��FOA��һ���ģ������Բ���Ҫ����ת��
		pNtHeaders = (PIMAGE_NT_HEADERS)(buffer + pDosHeader->e_lfanew);
		// ���PEͷ��ǩ��(0x4550)
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
		//��ʾ��32λPE�ļ�����64λPE�ļ� 10B32λ 20B64λ
		printf("Magic: %x\n", pOptionalHeader->Magic);

		delete[] buffer;
		CloseHandle(hfile);
		return 1;


	}

	delete[] buffer;
	CloseHandle(hfile);
	return 0;



}