#pragma once
#include<Windows.h>

typedef struct _PACKINFO {
	DWORD newOEP;
	DWORD oldOEP;
}PACKINFO, * PPACKINFO;

//_declspec(dllexport)：表示将变量 g_PackInfo 导出为 DLL 的全局变量，其他模块（比如 PE 加载器、脱壳工具等）可以从 DLL 中访问它。
//g_PackInfo：这个变量在运行时用于存储当前程序的 oldOEP 和 newOEP。
extern "C" _declspec(dllexport) PACKINFO g_PackInfo;

void packStart(); //加壳main
BOOL DecodeSections(); //解码代码段
//为了增强代码的隐蔽性和反分析能力，动态获取函数地址
//如果直接调用 LoadLibraryA、GetProcAddress 等函数，编译器会在 PE 文件的导入表中记录这些依赖关系。
//这样做会暴露壳代码的意图，让逆向工程师很容易发现壳代码使用了哪些 API 函数。
void GetFunctions(); //动态获得需要的函数的地址