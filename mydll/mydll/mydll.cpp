#include "mydll.h"

// DLL 入口点
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
    return a + b;  // 实际实现求和
}

int mydll::div(int a, int b) {
    if (b != 0) {
        return a / b;  // 实际实现除法，添加除零检查
    }
    return 0;
}

void mydll::fun1(char a, char c) {
    // 添加实际功能实现
}