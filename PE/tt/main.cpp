#include "mydll3.h"
int main() {
	int b = sum1(1, 2);
    // 动态加载 DLL
    HMODULE hMod = LoadLibraryA("mydll3.dll");
    if (!hMod) {
        std::cerr << "Failed to load mydll3.dll\n";
        return 1;
    }

    // 用序号导入无名导出函数（注意强转为 LPCSTR）
    NoNameFunc pTest11 = (NoNameFunc)GetProcAddress(hMod, (LPCSTR)2);
    NoNameFunc pTest22 = (NoNameFunc)GetProcAddress(hMod, (LPCSTR)3);
    NoNameFunc pFun2 = (NoNameFunc)GetProcAddress(hMod, (LPCSTR)4);

    if (pTest11) std::cout << "test11() = " << pTest11() << "\n";
    else std::cout << "test11() load failed\n";

    if (pTest22) std::cout << "test22() = " << pTest22() << "\n";
    else std::cout << "test22() load failed\n";

    if (pFun2) std::cout << "fun2() = " << pFun2() << "\n";
    else std::cout << "fun2() load failed\n";
	return 0;
}