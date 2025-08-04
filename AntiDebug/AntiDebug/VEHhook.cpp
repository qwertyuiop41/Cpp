#include <Windows.h>
#include <iostream>



typedef int (WINAPI *_MessageBoxA)(
	 HWND   hWnd,
	 LPCSTR lpText,
	 LPCSTR lpCaption,
	 UINT   uType
);

typedef struct _DR7
{
    unsigned L0 : 1;
    unsigned G0 : 1;
    unsigned L1 : 1;
    unsigned G1 : 1;
    unsigned L2 : 1;
    unsigned G2 : 1;
    unsigned L3 : 1;
    unsigned G3 : 1;

    unsigned LE : 1;
    unsigned GE : 1;

    unsigned Reserve1 : 3;

    unsigned GD : 1;

    unsigned Reserve2 : 2;

    unsigned RW0 : 2;
    unsigned LEN0 : 2;
    unsigned RW1 : 2;
    unsigned LEN1 : 2;
    unsigned RW2 : 2;
    unsigned LEN2 : 2;
    unsigned RW3 : 2;
    unsigned LEN3 : 2;
}DR7, * PDR7;


_MessageBoxA MyMessageBoxA = 0;


LONG WINAPI PvectoredExceptionHandler(
    _EXCEPTION_POINTERS* ExceptionInfo
)
{
    //MessageBoxA(0, "VEH�쳣�������ִ���ˣ�", "��ʾ", MB_OK);
    printf("VEH�쳣�������ִ���ˣ�");
    ExceptionInfo->ContextRecord->Eip = ExceptionInfo->ContextRecord->Eip + 6;
    system("pause");
    return EXCEPTION_CONTINUE_EXECUTION;
    //return EXCEPTION_CONTINUE_SEARCH;
}



//����һ��Ӳ���ϵ��� MessageBoxA ����������������VEH����öϵ㴥���¼���
//��Ϊû�е�������������VEH������SEH
int main() {
	DWORD breakpointAddress = 0;
	HMODULE hModule = GetModuleHandleA("user32.dll");
	MyMessageBoxA = (_MessageBoxA)GetProcAddress(hModule, "MessageBoxA");
	breakpointAddress = (DWORD)MyMessageBoxA;

	//set hardware breakpoint
	CONTEXT context{};
	context.Dr0 = breakpointAddress; //���öϵ��ַ
    // ׼�� Dr7 ����
    DR7 dr7 = {};
    dr7.L0 = 1;       // ���� Dr0
    dr7.RW0 = 0b00;   // ִ�жϵ�
    dr7.LEN0 = 0b00;  // ����Ϊ 1��ִ�жϵ����ӳ��ȣ��̶�Ϊ1��

    // ���ṹ��תΪ DWORD д�� context.Dr7
    memcpy(&context.Dr7, &dr7, sizeof(DWORD));

	SetThreadContext(GetCurrentThread(), &context);
    AddVectoredExceptionHandler(1, PvectoredExceptionHandler);

    MyMessageBoxA(0, "hello world", "��ʾ", MB_OK);
    system("pause");
    return 0;

}