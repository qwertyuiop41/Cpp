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
	PIMAGE_IMPORT_DESCRIPTOR pNewImportDescriptor; //�µ������ĵ�ַ
	DWORD RvaToFoa(DWORD rva);
	DWORD FoaToRva(DWORD foa); //��FOAת��ΪRVA

public:
	CPeUtil();
	~CPeUtil();
	BOOL LoadFile(const char* path); //���ر��ӿ�PE�ļ�
	BOOL InsertSection(const char* sectionName, DWORD codeSize, char* codebuff, DWORD dwCharacteristic);//��ӿհ�sections�����ƴ���κ��µ����
	BOOL InitFileInfo(); //��ʼ��PE�ļ������ӿǳ���/�ӿǺ���򣩵���Ϣ
	DWORD GetAlignmentSize(DWORD realSize, DWORD alignmentSize); //�����ı�/�ڴ����
	PIMAGE_SECTION_HEADER GetLastSection(); //���������Զ���section��section header��ַ
	BOOL SaveFile(const char* path);  //����+���ӿǳ��򹹳ɵ���PE�ļ����
	BOOL EncodeSections();  //���ܴ����
	DWORD GetOldOEP(); //��ȡ���ӿǳ����OEP
	BOOL SetOEP(DWORD newOEPRVA);  //����OEPΪ�Ǵ����OEP
	BOOL RepairRelocLocal(DWORD imageBase); //�޸��ض�λ���������ַ�棩��ֻ�޸��ض�λ��block�е�����
	BOOL RepairReloc(DWORD imageBase); //�޸��ض�λ�������ַ�棩���ض�λ���VirtualAddressҲ��Ҫ�޸�
	BOOL GetImportTable(); //��ȡ���������µ������ĵ�ַ
	


};

