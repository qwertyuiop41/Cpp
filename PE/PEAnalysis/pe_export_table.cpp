// pe_export_table.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include <windows.h>
#include <cstdio>

void* g_FileBuffer = 0;

DWORD g_pFileImageBase = 0;

char fileName[] = "C:/Documents and Settings/Administrator/����/aaaamon.dll";




//void _openFile(); 
DWORD RVAtoFOA(DWORD dwRVA)
{
	// ���DOSͷ����Ϣ
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_FileBuffer;
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);


	//��RVA�����ĸ�������
	//�ҵ��������κ�
	//��ȥ�������ε���ʼλ�ã��������ļ��е���ʼλ��
	//���ļ�ͷ���������� ����
	int nCountOfSection = pNtHeader->FileHeader.NumberOfSections;
	//���α�ͷ
	PIMAGE_SECTION_HEADER pSec = IMAGE_FIRST_SECTION(pNtHeader);
	//����չͷ���ҵ�������������ڴ�Ķ�������
	DWORD dwSecAligment = pNtHeader->OptionalHeader.SectionAlignment;
	//ѭ��
	for (int i = 0; i < nCountOfSection; i++)
	{
		//�����ڴ��е���ʵ��С
		//Misc.VirtualSize % dwSecAligment�����0����պö��������ȶ��루��0�����棩
		//Misc.VirtualSize / dwSecAligment * dwSecAligment   + dwSecAligment     //�����������Ķ���
		DWORD dwRealVirSize = pSec->Misc.VirtualSize % dwSecAligment ?
			pSec->Misc.VirtualSize / dwSecAligment * dwSecAligment + dwSecAligment
			: pSec->Misc.VirtualSize;


		//�����е���������ַת�ļ�ƫ��  ˼·�� ��Ҫת���ĵ�ַ�������
		//����ʼ��ַ���Ƚ��������һ�������У�������ʼ��ַС����ʼ��ַ���������ƫ�ƺͣ���
		//����Ҫת������������ַ��ȥ���ε���ʼ��ַ����������ַ��
		//�õ��������ַ����������ƫ�ƣ����õõ������ƫ�Ƽ����������ļ��е�ƫ�Ƶ���ʼλ��
		//��pointerToRawData�ֶ�)���������ļ��е��ļ�ƫ��
		if (dwRVA >= pSec->VirtualAddress &&
			dwRVA < pSec->VirtualAddress + dwRealVirSize)
		{
			//FOA = RVA - �ڴ������ε���ʼλ�� + ���ļ������ε���ʼλ�� 
			return dwRVA - pSec->VirtualAddress + pSec->PointerToRawData;
		}
		//��һ�����ε�ַ
		pSec++;
	}
}





void ReadDiskFile2FileBuffer(char* fileName, void** FileBuffer)
{
	//���ļ�����ȡPE�ļ�
	FILE* fileAddress = fopen(fileName, "rb");
	if (!fileAddress)
	{
		printf("���ļ�ʧ�ܣ���");
		return;
	}

	//����PE�ļ���С
	fseek(fileAddress, 0, SEEK_END);
	DWORD Filesize = ftell(fileAddress);
	printf("�ļ���СΪ%d\n", Filesize);
	//��ָ���ƶ�����ͷ��Ϊ�������ȡ�ļ�
	//����ʹ��rewind����
	fseek(fileAddress, 0, SEEK_SET);
	//����ѿռ�
	void* Temp_FileBuffer = malloc(Filesize);
	if (!Temp_FileBuffer)
	{
		printf("����Temp_FileBuffer�ռ�ʧ�ܣ���");
		free(Temp_FileBuffer);
		fclose(fileAddress);
		return;

	}
	memset(Temp_FileBuffer, 0, Filesize);

	//���ļ����ݶ�ȡ���ѿռ�
	size_t Terms = fread(Temp_FileBuffer, Filesize, 1, fileAddress);
	if (!Terms)
	{
		printf("����FileBufferʧ�ܣ�����");
		free(Temp_FileBuffer);
		fclose(fileAddress);
		return;
	}
	//�ر��ļ� ����Temp_FileBuffer��ֵ����FileBuffer

	*FileBuffer = Temp_FileBuffer;
	fclose(fileAddress);

}

void initFileBufferAndImageBase()
{

	void* FileBuffer = NULL;
	ReadDiskFile2FileBuffer(fileName, &FileBuffer);
	g_FileBuffer = FileBuffer;

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)FileBuffer;

	PIMAGE_OPTIONAL_HEADER32 pOptionsHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pDosHeader + pDosHeader->e_lfanew + 4 + 20); // ָ�������ҵ���ѡPEͷ
	g_pFileImageBase = pOptionsHeader->ImageBase;

	printf("ImageBase initFileBufferAndImageBase %08x\n", pOptionsHeader->ImageBase);

}


void output_pe_info()
{
	void* FileBuffer = NULL;

	ReadDiskFile2FileBuffer(fileName, &FileBuffer);

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)FileBuffer;

	PIMAGE_FILE_HEADER pPeHeader = (PIMAGE_FILE_HEADER)((DWORD)pDosHeader + pDosHeader->e_lfanew + 4);

	PIMAGE_OPTIONAL_HEADER32 pOptionsHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pPeHeader + 20);

	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pOptionsHeader + pPeHeader->SizeOfOptionalHeader);

	printf("%08x\n", pOptionsHeader->SizeOfCode);
	printf("�ڵ�����%08x\n", pPeHeader->NumberOfSections);
	printf("ImageBase output_pe_info %08x\n", pOptionsHeader->ImageBase);



}

void output_export_table()
{

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_FileBuffer;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);


	//�ҵ�������  Ҳ���ǵ�һ�����±�Ϊ0
	DWORD dwExportRVA = pNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress;
	//��ȡ���ļ��е�λ��
	PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)(RVAtoFOA(dwExportRVA) + pNtHeaders->OptionalHeader.ImageBase);
	//ģ������
	//DWORD name = (RVAtoFOA(pExport->Name) + g_pFileImageBase);
	//printf("%08x\n", name);
	//��ַ���еĸ���
	DWORD dwCountOfFuntions = pExport->NumberOfFunctions;
	//���Ʊ��еĸ���
	DWORD dwCountOfNames = pExport->NumberOfNames;
	//��ַ���ַ
	PDWORD pAddrOfFuntion = (PDWORD)(RVAtoFOA(pExport->AddressOfFunctions) + g_pFileImageBase);
	//���Ʊ��ַ
	PDWORD pAddrOfName = (PDWORD)(RVAtoFOA(pExport->AddressOfNames) + g_pFileImageBase);
	//��ű��ַ
	PWORD pAddrOfOrdial = (PWORD)(RVAtoFOA(pExport->AddressOfNameOrdinals) + g_pFileImageBase);
	//baseֵ
	DWORD dwBase = pExport->Base;
	//������ַ���е�Ԫ��
	printf("-----------------------------------------�������еĵ��������뵼�����-------------------------------------------------- \r\n");
	if (dwExportRVA == 0) {
		printf("û�е�����\n");
		//return;
	}
	else {
		int i = 0;
		for (; i < dwCountOfFuntions; i++)
		{
			//��ַ���п��ܴ������õ�ֵ������Ϊ0��ֵ��
			if (pAddrOfFuntion[i] == 0)
			{
				continue;
			}
			//������ű����Ƿ���ֵ����ַ����±�ֵ����
			//���ж��Ƿ������Ƶ���
			bool bRet = false;
			int j = 0;
			for (; j < dwCountOfNames; j++)
			{
				//iΪ��ַ���±�jΪ��ű���±ֵ꣨Ϊ��ַ���±꣩
				//�ж��Ƿ�����ű���
				if (i == pAddrOfOrdial[j])
				{
					//��Ϊ��ű������Ʊ��λ��һһ��Ӧ
					//ȡ�����Ʊ��е����Ƶ�ַRVA
					DWORD dwNameRVA = pAddrOfName[j];
					char* pFunName = (char*)(RVAtoFOA(dwNameRVA) + g_pFileImageBase);
					printf("%04d  %s  0x%08x\n", i + dwBase, pFunName, pAddrOfFuntion[i]);
					bRet = true;
					break;
				}
			}
			if (!bRet)
			{
				//��ű���û�У�˵��������ŵ�����
				printf("%04d           %08X\n", i + dwBase, pAddrOfFuntion[i]);
			}

		}
	}
}



/*
1����ӡ��������Ϣ������ӡ��������ַ������������ű�

2��ͬʱд�������ֲ��Һ�����ַ������Ų��Һ�����ַ��غ�����
*/


/*
int PrintFunctionAddressTable(PVOID FileAddress, DWORD AddressOfFunctions_RVA, DWORD NumberOfFunctions)
{
	int ret = 0;
	DWORD AddressOfFunctions_FOA = 0;

	//1��RVA --> FOA
	ret = RVA_TO_FOA(FileAddress, AddressOfFunctions_RVA, &AddressOfFunctions_FOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//2��ָ������ַ��
	PDWORD FuncAddressTable = (PDWORD)((DWORD)FileAddress + AddressOfFunctions_FOA);

	//2��ѭ����ӡ������ַ��
	printf("=================== ������ַ�� Start ===================\n");
	for (DWORD i = 0; i < NumberOfFunctions; i++)
	{
		DWORD FuncAddress_RVA = FuncAddressTable[i];
		DWORD FuncAddress_FOA = 0;
		ret = RVA_TO_FOA(FileAddress, FuncAddress_RVA, &FuncAddress_FOA);
		if (ret != 0)
		{
			printf("func RVA_TO_FOA() Error!\n");
			return ret;
		}

		printf("������ַRVA    : %08X  |������ַFOA    : %08X  \n", FuncAddress_RVA, FuncAddress_FOA);
	}
	printf("=================== ������ַ�� End   ===================\n\n");
	return ret;
}
*/

/*
//��ӡ������ű�
int PrintFunctionOrdinalTable(PVOID FileAddress, DWORD AddressOfOrdinal_RVA, DWORD NumberOfNames, DWORD Base)
{
	int ret = 0;
	DWORD AddressOfOrdinal_FOA = 0;

	//1��RVA --> FOA
	ret = RVA_TO_FOA(FileAddress, AddressOfOrdinal_RVA, &AddressOfOrdinal_FOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//2��ָ������ű�
	PWORD OrdinalTable = (PWORD)((DWORD)FileAddress + AddressOfOrdinal_FOA);

	//3��ѭ����ӡ������ű�
	printf("=================== ������ű� Start ===================\n");
	for (DWORD i = 0; i < NumberOfNames; i++)
	{
		printf("�������  :%04X  |Base+Ordinal   :%04X\n", OrdinalTable[i], OrdinalTable[i] + Base);
	}
	printf("=================== ������ű� End   ===================\n\n");
	return ret;
}


int PrintFunctionNameTable(PVOID FileAddress, DWORD AddressOfNames_RVA, DWORD NumberOfNames)
{
	int ret = 0;
	DWORD AddressOfNames_FOA = 0;

	//1��RVA --> FOA
	ret = RVA_TO_FOA(FileAddress, AddressOfNames_RVA, &AddressOfNames_FOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//2��ָ��������
	PDWORD NameTable = (PDWORD)((DWORD)FileAddress + AddressOfNames_FOA);

	//3��ѭ����ӡ������ű�
	printf("=================== �������� Start ===================\n");
	for (DWORD i = 0; i < NumberOfNames; i++)
	{
		DWORD FuncName_RVA = NameTable[i];
		DWORD FuncName_FOA = 0;
		ret = RVA_TO_FOA(FileAddress, FuncName_RVA, &FuncName_FOA);
		if (ret != 0)
		{
			printf("func RVA_TO_FOA() Error!\n");
			return ret;
		}
		PCHAR FuncName = (PCHAR)((DWORD)FileAddress + FuncName_FOA);

		printf("������  :%s\n", FuncName);
	}
	printf("=================== �������� End   ===================\n\n");

	return ret;
}


int PrintExportTable(PVOID FileAddress)
{
	int ret = 0;

	//1��ָ���������
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)(FileAddress);
	PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)((DWORD)pDosHeader + pDosHeader->e_lfanew + 4);
	PIMAGE_OPTIONAL_HEADER32 pOptionalHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pFileHeader + sizeof(IMAGE_FILE_HEADER));

	//2����ȡ������ĵ�ַ
	DWORD ExportDirectory_RAVAdd = pOptionalHeader->DataDirectory[0].VirtualAddress;
	DWORD ExportDirectory_FOAAdd = 0;
	//	(1)���жϵ������Ƿ����
	if (ExportDirectory_RAVAdd == 0)
	{
		printf("ExportDirectory ������!\n");
		return ret;
	}
	//	(2)����ȡ�������FOA��ַ
	ret = RVA_TO_FOA(FileAddress, ExportDirectory_RAVAdd, &ExportDirectory_FOAAdd);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//3��ָ�򵼳���
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)FileAddress + ExportDirectory_FOAAdd);

	//4���ҵ��ļ���
	DWORD FileName_RVA = ExportDirectory->Name;
	DWORD FileName_FOA = 0;
	ret = RVA_TO_FOA(FileAddress, FileName_RVA, &FileName_FOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}
	PCHAR FileName = (PCHAR)((DWORD)FileAddress + FileName_FOA);

	//5����ӡ��������Ϣ
	printf("DWORD Characteristics;        :  %08X\n", ExportDirectory->Characteristics);
	printf("DWORD TimeDateStamp;          :  %08X\n", ExportDirectory->TimeDateStamp);
	printf("WORD  MajorVersion;           :  %04X\n", ExportDirectory->MajorVersion);
	printf("WORD  MinorVersion;           :  %04X\n", ExportDirectory->MinorVersion);
	printf("DWORD Name;                   :  %08X     \"%s\"\n", ExportDirectory->Name, FileName);
	printf("DWORD Base;                   :  %08X\n", ExportDirectory->Base);
	printf("DWORD NumberOfFunctions;      :  %08X\n", ExportDirectory->NumberOfFunctions);
	printf("DWORD NumberOfNames;          :  %08X\n", ExportDirectory->NumberOfNames);
	printf("DWORD AddressOfFunctions;     :  %08X\n", ExportDirectory->AddressOfFunctions);
	printf("DWORD AddressOfNames;         :  %08X\n", ExportDirectory->AddressOfNames);
	printf("DWORD AddressOfNameOrdinals;  :  %08X\n", ExportDirectory->AddressOfNameOrdinals);
	printf("=========================================================\n");
	printf("*********************************************************\n");

	//6����ӡ������ַ�� ������NumberOfFunctions����
	ret = PrintFunctionAddressTable(FileAddress, ExportDirectory->AddressOfFunctions, ExportDirectory->NumberOfFunctions);
	if (ret != 0)
	{
		printf("func PrintFunctionAddressTable() Error!\n");
		return ret;
	}

	//7����ӡ������ű� ������NumberOfNames����
	ret = PrintFunctionOrdinalTable(FileAddress, ExportDirectory->AddressOfNameOrdinals, ExportDirectory->NumberOfNames, ExportDirectory->Base);
	if (ret != 0)
	{
		printf("func PrintFunctionOrdinalTable() Error!\n");
		return ret;
	}

	//8����ӡ�������� ������NumberOfNames����
	ret = PrintFunctionNameTable(FileAddress, ExportDirectory->AddressOfNames, ExportDirectory->NumberOfNames);
	if (ret != 0)
	{
		printf("func PrintFunctionNameTable() Error!\n");
		return ret;
	}

	return ret;
}

//===============================================================================================

int GetProcAddressByName(PVOID FileAddress, PCHAR pFuncName, PDWORD FuncAddressRVA)
{
	int ret = 0;

	//1��ָ���������
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)(FileAddress);
	PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)((DWORD)pDosHeader + pDosHeader->e_lfanew + 4);
	PIMAGE_OPTIONAL_HEADER32 pOptionalHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pFileHeader + sizeof(IMAGE_FILE_HEADER));

	//2����ȡ������ĵ�ַ
	DWORD ExportDirectory_RAVAdd = pOptionalHeader->DataDirectory[0].VirtualAddress;
	DWORD ExportDirectory_FOAAdd = 0;
	//	(1)���жϵ������Ƿ����
	if (ExportDirectory_RAVAdd == 0)
	{
		printf("ExportDirectory ������!\n");
		return ret;
	}
	//	(2)����ȡ�������FOA��ַ
	ret = RVA_TO_FOA(FileAddress, ExportDirectory_RAVAdd, &ExportDirectory_FOAAdd);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//3��ָ�򵼳���
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)FileAddress + ExportDirectory_FOAAdd);

	//4��ָ��������
	DWORD FuncNameTableRVA = ExportDirectory->AddressOfNames;
	DWORD FuncNameTableFOA = 0;
	ret = RVA_TO_FOA(FileAddress, FuncNameTableRVA, &FuncNameTableFOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}
	PDWORD FuncNameTable = (PDWORD)((DWORD)FileAddress + FuncNameTableFOA);

	//5��������������
	for (DWORD i = 0; i < ExportDirectory->NumberOfNames; i++)
	{
		DWORD FuncNameRVA = FuncNameTable[i];
		DWORD FuncNameFOA = 0;
		ret = RVA_TO_FOA(FileAddress, FuncNameRVA, &FuncNameFOA);
		if (ret != 0)
		{
			printf("func RVA_TO_FOA() Error!\n");
			return ret;
		}
		PCHAR FuncName = (PCHAR)((DWORD)FileAddress + FuncNameFOA);

		//6���ҵ���ͬ�ĺ�����
		if (memcmp(FuncName, pFuncName, strlen(pFuncName)) == 0)
		{
			//7�����������ĺ�����ŵ�����ֵ
			DWORD dwFuncOrdinalIndex = i;

			//8���ҵ�������ű�
			DWORD FuncOrdinalTableRVA = ExportDirectory->AddressOfNameOrdinals;
			DWORD FuncOrdinalTableFOA = 0;
			ret = RVA_TO_FOA(FileAddress, FuncOrdinalTableRVA, &FuncOrdinalTableFOA);
			if (ret != 0)
			{
				printf("func RVA_TO_FOA() Error!\n");
				return ret;
			}
			PWORD FuncOrdinalTable = (PWORD)((DWORD)FileAddress + FuncOrdinalTableFOA);

			//9����ȡ�������
			WORD wFuncOrdinal = FuncOrdinalTable[dwFuncOrdinalIndex];

			//10���ҵ�������ַ��
			DWORD FuncAddressTableRVA = ExportDirectory->AddressOfFunctions;
			DWORD FuncAddressTableFOA = 0;
			ret = RVA_TO_FOA(FileAddress, FuncAddressTableRVA, &FuncAddressTableFOA);
			if (ret != 0)
			{
				printf("func RVA_TO_FOA() Error!\n");
				return ret;
			}
			PDWORD FuncAddressTable = (PDWORD)((DWORD)FileAddress + FuncAddressTableFOA);

			//11����ȡ������ַ
			*FuncAddressRVA = FuncAddressTable[wFuncOrdinal];

			break;
		}
	}

	return ret;
}


int GetProcAddressByOrdinal(PVOID FileAddress, WORD wFuncOrdinal, PDWORD FuncAddressRVA)
{
	int ret = 0;

	//1��ָ���������
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)(FileAddress);
	PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)((DWORD)pDosHeader + pDosHeader->e_lfanew + 4);
	PIMAGE_OPTIONAL_HEADER32 pOptionalHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pFileHeader + sizeof(IMAGE_FILE_HEADER));

	//2����ȡ������ĵ�ַ
	DWORD ExportDirectory_RAVAdd = pOptionalHeader->DataDirectory[0].VirtualAddress;
	DWORD ExportDirectory_FOAAdd = 0;
	//	(1)���жϵ������Ƿ����
	if (ExportDirectory_RAVAdd == 0)
	{
		printf("ExportDirectory ������!\n");
		return ret;
	}
	//	(2)����ȡ�������FOA��ַ
	ret = RVA_TO_FOA(FileAddress, ExportDirectory_RAVAdd, &ExportDirectory_FOAAdd);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//3��ָ�򵼳���
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)FileAddress + ExportDirectory_FOAAdd);

	//4����ȡ�������
	DWORD FuncOrdinal = wFuncOrdinal - ExportDirectory->Base;

	//5���ҵ�������ַ��
	DWORD FuncAddressTableRVA = ExportDirectory->AddressOfFunctions;
	DWORD FuncAddressTableFOA = 0;
	ret = RVA_TO_FOA(FileAddress, FuncAddressTableRVA, &FuncAddressTableFOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}
	PDWORD FuncAddressTable = (PDWORD)((DWORD)FileAddress + FuncAddressTableFOA);

	//11����ȡ������ַ
	*FuncAddressRVA = FuncAddressTable[FuncOrdinal];

	return ret;
}

  */
int main(int argc, char* argv[])
{
	//output_pe_info();
	//output_pe_info();
	initFileBufferAndImageBase();
	output_export_table();

	printf("Hello World!\n");
	return 0;
}

