#pragma once
#include<Windows.h>

typedef struct _PACKINFO {
	DWORD newOEP;
	DWORD oldOEP;
}PACKINFO, * PPACKINFO;

//_declspec(dllexport)����ʾ������ g_PackInfo ����Ϊ DLL ��ȫ�ֱ���������ģ�飨���� PE ���������ѿǹ��ߵȣ����Դ� DLL �з�������
//g_PackInfo���������������ʱ���ڴ洢��ǰ����� oldOEP �� newOEP��
extern "C" _declspec(dllexport) PACKINFO g_PackInfo;

void packStart(); //�ӿ�main
BOOL DecodeSections(); //��������
//Ϊ����ǿ����������Ժͷ�������������̬��ȡ������ַ
//���ֱ�ӵ��� LoadLibraryA��GetProcAddress �Ⱥ��������������� PE �ļ��ĵ�����м�¼��Щ������ϵ��
//�������ᱩ¶�Ǵ������ͼ�������򹤳�ʦ�����׷��ֿǴ���ʹ������Щ API ������
void GetFunctions(); //��̬�����Ҫ�ĺ����ĵ�ַ