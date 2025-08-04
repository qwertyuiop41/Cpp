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
    //MessageBoxA(0, "VEH异常处理代码执行了！", "提示", MB_OK);
    printf("VEH异常处理代码执行了！");
    ExceptionInfo->ContextRecord->Eip = ExceptionInfo->ContextRecord->Eip + 6;
    system("pause");
    return EXCEPTION_CONTINUE_EXECUTION;
    //return EXCEPTION_CONTINUE_SEARCH;
}



//设置一个硬件断点在 MessageBoxA 函数处，并尝试用VEH捕获该断点触发事件。
//因为没有调试器，所以先VEH，再是SEH
int main() {
	DWORD breakpointAddress = 0;
	HMODULE hModule = GetModuleHandleA("user32.dll");
	MyMessageBoxA = (_MessageBoxA)GetProcAddress(hModule, "MessageBoxA");
	breakpointAddress = (DWORD)MyMessageBoxA;

	//set hardware breakpoint
	CONTEXT context{};
	context.Dr0 = breakpointAddress; //设置断点地址
    // 准备 Dr7 设置
    DR7 dr7 = {};
    dr7.L0 = 1;       // 启用 Dr0
    dr7.RW0 = 0b00;   // 执行断点
    dr7.LEN0 = 0b00;  // 长度为 1（执行断点无视长度，固定为1）

    // 将结构体转为 DWORD 写入 context.Dr7
    memcpy(&context.Dr7, &dr7, sizeof(DWORD));

	SetThreadContext(GetCurrentThread(), &context);
    AddVectoredExceptionHandler(1, PvectoredExceptionHandler);

    MyMessageBoxA(0, "hello world", "提示", MB_OK);
    system("pause");
    return 0;

}