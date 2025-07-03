// pe_export_table.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include <windows.h>
#include <cstdio>

void* g_FileBuffer = 0;

DWORD g_pFileImageBase = 0;

char fileName[] = "C:/Documents and Settings/Administrator/桌面/aaaamon.dll";




//void _openFile(); 
DWORD RVAtoFOA(DWORD dwRVA)
{
	// 获得DOS头等信息
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_FileBuffer;
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);


	//此RVA落在哪个区段中
	//找到所在区段后，
	//减去所在区段的起始位置，加上在文件中的起始位置
	//大文件头中找区段数 节数
	int nCountOfSection = pNtHeader->FileHeader.NumberOfSections;
	//区段表头
	PIMAGE_SECTION_HEADER pSec = IMAGE_FIRST_SECTION(pNtHeader);
	//在扩展头中找到块对齐数，在内存的对齐整数
	DWORD dwSecAligment = pNtHeader->OptionalHeader.SectionAlignment;
	//循环
	for (int i = 0; i < nCountOfSection; i++)
	{
		//求在内存中的真实大小
		//Misc.VirtualSize % dwSecAligment如果是0代表刚好对齐否则就先对齐（非0就是真）
		//Misc.VirtualSize / dwSecAligment * dwSecAligment   + dwSecAligment     //最后加上余数的对齐
		DWORD dwRealVirSize = pSec->Misc.VirtualSize % dwSecAligment ?
			pSec->Misc.VirtualSize / dwSecAligment * dwSecAligment + dwSecAligment
			: pSec->Misc.VirtualSize;


		//区段中的相对虚拟地址转文件偏移  思路是 用要转换的地址与各个区
		//段起始地址做比较如果落在一个区段中（大于起始地址小于起始地址加区段最大偏移和），
		//就用要转换的相对虚拟地址减去区段的起始地址的相对虚拟地址，
		//得到了这个地址相对这个区段偏移，再用得到的这个偏移加上区段在文件中的偏移的起始位置
		//（pointerToRawData字段)就是他在文件中的文件偏移
		if (dwRVA >= pSec->VirtualAddress &&
			dwRVA < pSec->VirtualAddress + dwRealVirSize)
		{
			//FOA = RVA - 内存中区段的起始位置 + 在文件中区段的起始位置 
			return dwRVA - pSec->VirtualAddress + pSec->PointerToRawData;
		}
		//下一个区段地址
		pSec++;
	}
}





void ReadDiskFile2FileBuffer(char* fileName, void** FileBuffer)
{
	//打开文件，读取PE文件
	FILE* fileAddress = fopen(fileName, "rb");
	if (!fileAddress)
	{
		printf("打开文件失败！！");
		return;
	}

	//计算PE文件大小
	fseek(fileAddress, 0, SEEK_END);
	DWORD Filesize = ftell(fileAddress);
	printf("文件大小为%d\n", Filesize);
	//将指针移动到开头，为了下面读取文件
	//或者使用rewind函数
	fseek(fileAddress, 0, SEEK_SET);
	//申请堆空间
	void* Temp_FileBuffer = malloc(Filesize);
	if (!Temp_FileBuffer)
	{
		printf("分配Temp_FileBuffer空间失败！！");
		free(Temp_FileBuffer);
		fclose(fileAddress);
		return;

	}
	memset(Temp_FileBuffer, 0, Filesize);

	//将文件内容读取到堆空间
	size_t Terms = fread(Temp_FileBuffer, Filesize, 1, fileAddress);
	if (!Terms)
	{
		printf("读入FileBuffer失败！！！");
		free(Temp_FileBuffer);
		fclose(fileAddress);
		return;
	}
	//关闭文件 ，将Temp_FileBuffer的值传给FileBuffer

	*FileBuffer = Temp_FileBuffer;
	fclose(fileAddress);

}

void initFileBufferAndImageBase()
{

	void* FileBuffer = NULL;
	ReadDiskFile2FileBuffer(fileName, &FileBuffer);
	g_FileBuffer = FileBuffer;

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)FileBuffer;

	PIMAGE_OPTIONAL_HEADER32 pOptionsHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pDosHeader + pDosHeader->e_lfanew + 4 + 20); // 指针运算找到可选PE头
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
	printf("节的数量%08x\n", pPeHeader->NumberOfSections);
	printf("ImageBase output_pe_info %08x\n", pOptionsHeader->ImageBase);



}

void output_export_table()
{

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_FileBuffer;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((DWORD)pDosHeader + pDosHeader->e_lfanew);


	//找到导出表  也就是第一个表下标为0
	DWORD dwExportRVA = pNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress;
	//获取在文件中的位置
	PIMAGE_EXPORT_DIRECTORY pExport = (PIMAGE_EXPORT_DIRECTORY)(RVAtoFOA(dwExportRVA) + pNtHeaders->OptionalHeader.ImageBase);
	//模块名字
	//DWORD name = (RVAtoFOA(pExport->Name) + g_pFileImageBase);
	//printf("%08x\n", name);
	//地址表中的个数
	DWORD dwCountOfFuntions = pExport->NumberOfFunctions;
	//名称表中的个数
	DWORD dwCountOfNames = pExport->NumberOfNames;
	//地址表地址
	PDWORD pAddrOfFuntion = (PDWORD)(RVAtoFOA(pExport->AddressOfFunctions) + g_pFileImageBase);
	//名称表地址
	PDWORD pAddrOfName = (PDWORD)(RVAtoFOA(pExport->AddressOfNames) + g_pFileImageBase);
	//序号表地址
	PWORD pAddrOfOrdial = (PWORD)(RVAtoFOA(pExport->AddressOfNameOrdinals) + g_pFileImageBase);
	//base值
	DWORD dwBase = pExport->Base;
	//遍历地址表中的元素
	printf("-----------------------------------------导出表中的导出函数与导出序号-------------------------------------------------- \r\n");
	if (dwExportRVA == 0) {
		printf("没有导出表\n");
		//return;
	}
	else {
		int i = 0;
		for (; i < dwCountOfFuntions; i++)
		{
			//地址表中可能存在无用的值（就是为0的值）
			if (pAddrOfFuntion[i] == 0)
			{
				continue;
			}
			//根据序号表中是否有值（地址表的下标值），
			//来判断是否是名称导出
			bool bRet = false;
			int j = 0;
			for (; j < dwCountOfNames; j++)
			{
				//i为地址表下标j为序号表的下标（值为地址表下标）
				//判断是否在序号表中
				if (i == pAddrOfOrdial[j])
				{
					//因为序号表与名称表的位置一一对应
					//取出名称表中的名称地址RVA
					DWORD dwNameRVA = pAddrOfName[j];
					char* pFunName = (char*)(RVAtoFOA(dwNameRVA) + g_pFileImageBase);
					printf("%04d  %s  0x%08x\n", i + dwBase, pFunName, pAddrOfFuntion[i]);
					bRet = true;
					break;
				}
			}
			if (!bRet)
			{
				//序号表中没有，说明是以序号导出的
				printf("%04d           %08X\n", i + dwBase, pAddrOfFuntion[i]);
			}

		}
	}
}



/*
1、打印导出表信息，并打印出函数地址表、函数名表、序号表

2、同时写出按名字查找函数地址、按序号查找函数地址相关函数。
*/


/*
int PrintFunctionAddressTable(PVOID FileAddress, DWORD AddressOfFunctions_RVA, DWORD NumberOfFunctions)
{
	int ret = 0;
	DWORD AddressOfFunctions_FOA = 0;

	//1、RVA --> FOA
	ret = RVA_TO_FOA(FileAddress, AddressOfFunctions_RVA, &AddressOfFunctions_FOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//2、指向函数地址表
	PDWORD FuncAddressTable = (PDWORD)((DWORD)FileAddress + AddressOfFunctions_FOA);

	//2、循环打印函数地址表
	printf("=================== 函数地址表 Start ===================\n");
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

		printf("函数地址RVA    : %08X  |函数地址FOA    : %08X  \n", FuncAddress_RVA, FuncAddress_FOA);
	}
	printf("=================== 函数地址表 End   ===================\n\n");
	return ret;
}
*/

/*
//打印函数序号表
int PrintFunctionOrdinalTable(PVOID FileAddress, DWORD AddressOfOrdinal_RVA, DWORD NumberOfNames, DWORD Base)
{
	int ret = 0;
	DWORD AddressOfOrdinal_FOA = 0;

	//1、RVA --> FOA
	ret = RVA_TO_FOA(FileAddress, AddressOfOrdinal_RVA, &AddressOfOrdinal_FOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//2、指向函数序号表
	PWORD OrdinalTable = (PWORD)((DWORD)FileAddress + AddressOfOrdinal_FOA);

	//3、循环打印函数序号表
	printf("=================== 函数序号表 Start ===================\n");
	for (DWORD i = 0; i < NumberOfNames; i++)
	{
		printf("函数序号  :%04X  |Base+Ordinal   :%04X\n", OrdinalTable[i], OrdinalTable[i] + Base);
	}
	printf("=================== 函数序号表 End   ===================\n\n");
	return ret;
}


int PrintFunctionNameTable(PVOID FileAddress, DWORD AddressOfNames_RVA, DWORD NumberOfNames)
{
	int ret = 0;
	DWORD AddressOfNames_FOA = 0;

	//1、RVA --> FOA
	ret = RVA_TO_FOA(FileAddress, AddressOfNames_RVA, &AddressOfNames_FOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//2、指向函数名表
	PDWORD NameTable = (PDWORD)((DWORD)FileAddress + AddressOfNames_FOA);

	//3、循环打印函数序号表
	printf("=================== 函数名表 Start ===================\n");
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

		printf("函数名  :%s\n", FuncName);
	}
	printf("=================== 函数名表 End   ===================\n\n");

	return ret;
}


int PrintExportTable(PVOID FileAddress)
{
	int ret = 0;

	//1、指向相关内容
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)(FileAddress);
	PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)((DWORD)pDosHeader + pDosHeader->e_lfanew + 4);
	PIMAGE_OPTIONAL_HEADER32 pOptionalHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pFileHeader + sizeof(IMAGE_FILE_HEADER));

	//2、获取导出表的地址
	DWORD ExportDirectory_RAVAdd = pOptionalHeader->DataDirectory[0].VirtualAddress;
	DWORD ExportDirectory_FOAAdd = 0;
	//	(1)、判断导出表是否存在
	if (ExportDirectory_RAVAdd == 0)
	{
		printf("ExportDirectory 不存在!\n");
		return ret;
	}
	//	(2)、获取导出表的FOA地址
	ret = RVA_TO_FOA(FileAddress, ExportDirectory_RAVAdd, &ExportDirectory_FOAAdd);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//3、指向导出表
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)FileAddress + ExportDirectory_FOAAdd);

	//4、找到文件名
	DWORD FileName_RVA = ExportDirectory->Name;
	DWORD FileName_FOA = 0;
	ret = RVA_TO_FOA(FileAddress, FileName_RVA, &FileName_FOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}
	PCHAR FileName = (PCHAR)((DWORD)FileAddress + FileName_FOA);

	//5、打印导出表信息
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

	//6、打印函数地址表 数量由NumberOfFunctions决定
	ret = PrintFunctionAddressTable(FileAddress, ExportDirectory->AddressOfFunctions, ExportDirectory->NumberOfFunctions);
	if (ret != 0)
	{
		printf("func PrintFunctionAddressTable() Error!\n");
		return ret;
	}

	//7、打印函数序号表 数量由NumberOfNames决定
	ret = PrintFunctionOrdinalTable(FileAddress, ExportDirectory->AddressOfNameOrdinals, ExportDirectory->NumberOfNames, ExportDirectory->Base);
	if (ret != 0)
	{
		printf("func PrintFunctionOrdinalTable() Error!\n");
		return ret;
	}

	//8、打印函数名表 数量由NumberOfNames决定
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

	//1、指向相关内容
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)(FileAddress);
	PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)((DWORD)pDosHeader + pDosHeader->e_lfanew + 4);
	PIMAGE_OPTIONAL_HEADER32 pOptionalHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pFileHeader + sizeof(IMAGE_FILE_HEADER));

	//2、获取导出表的地址
	DWORD ExportDirectory_RAVAdd = pOptionalHeader->DataDirectory[0].VirtualAddress;
	DWORD ExportDirectory_FOAAdd = 0;
	//	(1)、判断导出表是否存在
	if (ExportDirectory_RAVAdd == 0)
	{
		printf("ExportDirectory 不存在!\n");
		return ret;
	}
	//	(2)、获取导出表的FOA地址
	ret = RVA_TO_FOA(FileAddress, ExportDirectory_RAVAdd, &ExportDirectory_FOAAdd);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//3、指向导出表
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)FileAddress + ExportDirectory_FOAAdd);

	//4、指向函数名表
	DWORD FuncNameTableRVA = ExportDirectory->AddressOfNames;
	DWORD FuncNameTableFOA = 0;
	ret = RVA_TO_FOA(FileAddress, FuncNameTableRVA, &FuncNameTableFOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}
	PDWORD FuncNameTable = (PDWORD)((DWORD)FileAddress + FuncNameTableFOA);

	//5、遍历函数名表
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

		//6、找到相同的函数名
		if (memcmp(FuncName, pFuncName, strlen(pFuncName)) == 0)
		{
			//7、计算真正的函数序号的索引值
			DWORD dwFuncOrdinalIndex = i;

			//8、找到函数序号表
			DWORD FuncOrdinalTableRVA = ExportDirectory->AddressOfNameOrdinals;
			DWORD FuncOrdinalTableFOA = 0;
			ret = RVA_TO_FOA(FileAddress, FuncOrdinalTableRVA, &FuncOrdinalTableFOA);
			if (ret != 0)
			{
				printf("func RVA_TO_FOA() Error!\n");
				return ret;
			}
			PWORD FuncOrdinalTable = (PWORD)((DWORD)FileAddress + FuncOrdinalTableFOA);

			//9、获取函数序号
			WORD wFuncOrdinal = FuncOrdinalTable[dwFuncOrdinalIndex];

			//10、找到函数地址表
			DWORD FuncAddressTableRVA = ExportDirectory->AddressOfFunctions;
			DWORD FuncAddressTableFOA = 0;
			ret = RVA_TO_FOA(FileAddress, FuncAddressTableRVA, &FuncAddressTableFOA);
			if (ret != 0)
			{
				printf("func RVA_TO_FOA() Error!\n");
				return ret;
			}
			PDWORD FuncAddressTable = (PDWORD)((DWORD)FileAddress + FuncAddressTableFOA);

			//11、获取函数地址
			*FuncAddressRVA = FuncAddressTable[wFuncOrdinal];

			break;
		}
	}

	return ret;
}


int GetProcAddressByOrdinal(PVOID FileAddress, WORD wFuncOrdinal, PDWORD FuncAddressRVA)
{
	int ret = 0;

	//1、指向相关内容
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)(FileAddress);
	PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)((DWORD)pDosHeader + pDosHeader->e_lfanew + 4);
	PIMAGE_OPTIONAL_HEADER32 pOptionalHeader = (PIMAGE_OPTIONAL_HEADER32)((DWORD)pFileHeader + sizeof(IMAGE_FILE_HEADER));

	//2、获取导出表的地址
	DWORD ExportDirectory_RAVAdd = pOptionalHeader->DataDirectory[0].VirtualAddress;
	DWORD ExportDirectory_FOAAdd = 0;
	//	(1)、判断导出表是否存在
	if (ExportDirectory_RAVAdd == 0)
	{
		printf("ExportDirectory 不存在!\n");
		return ret;
	}
	//	(2)、获取导出表的FOA地址
	ret = RVA_TO_FOA(FileAddress, ExportDirectory_RAVAdd, &ExportDirectory_FOAAdd);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}

	//3、指向导出表
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((DWORD)FileAddress + ExportDirectory_FOAAdd);

	//4、获取函数序号
	DWORD FuncOrdinal = wFuncOrdinal - ExportDirectory->Base;

	//5、找到函数地址表
	DWORD FuncAddressTableRVA = ExportDirectory->AddressOfFunctions;
	DWORD FuncAddressTableFOA = 0;
	ret = RVA_TO_FOA(FileAddress, FuncAddressTableRVA, &FuncAddressTableFOA);
	if (ret != 0)
	{
		printf("func RVA_TO_FOA() Error!\n");
		return ret;
	}
	PDWORD FuncAddressTable = (PDWORD)((DWORD)FileAddress + FuncAddressTableFOA);

	//11、获取函数地址
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

