#include "mydll.h"

// DLL ��ڵ�
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

int mydll::sum(int a, int b) {
    return a + b;  // ʵ��ʵ�����
}

int mydll::div(int a, int b) {
    if (b != 0) {
        return a / b;  // ʵ��ʵ�ֳ�������ӳ�����
    }
    return 0;
}

void mydll::fun1(char a, char c) {
    // ���ʵ�ʹ���ʵ��
}