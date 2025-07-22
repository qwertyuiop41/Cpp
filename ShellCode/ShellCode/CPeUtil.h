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
	DWORD importTableSize;
	PIMAGE_IMPORT_DESCRIPTOR pNewImportDescriptor; //新导入表保存的地址
	DWORD RvaToFoa(DWORD rva);
	DWORD FoaToRva(DWORD foa); //将FOA转换为RVA
	
public:
	CPeUtil();
	~CPeUtil();
	BOOL LoadFile(const char* path); //加载被加壳PE文件
	BOOL InsertSection(const char* sectionName, DWORD codeSize,  char* codebuff, DWORD dwCharacteristic);//添加空白sections（复制代码段和新导入表）
	BOOL InitFileInfo(); //初始化PE文件（被加壳程序/加壳后程序）的信息
	DWORD GetAlignmentSize(DWORD realSize,DWORD alignmentSize); //根据文本/内存对齐
	PIMAGE_SECTION_HEADER GetLastSection(); //获得新添的自定义section的section header地址
	BOOL SaveFile(const char* path);  //将壳+被加壳程序构成的新PE文件另存
	BOOL EncodeSections();  //加密代码段
	DWORD GetOldOEP(); //获取被加壳程序的OEP
	BOOL SetOEP(DWORD newOEPRVA);  //设置OEP为壳代码的OEP
	BOOL RepairReloc(DWORD imageBase); //壳代码注入后，windows不会自动修复重定位表，会导致全局变量失效，所以要自己手动修复
	BOOL GetImportTable(); //获取导入表并获得新导入表保存的地址
	

};

