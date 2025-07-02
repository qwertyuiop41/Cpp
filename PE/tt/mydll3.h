
#pragma once
#pragma comment(lib ,"mydll3.lib")

#include <windows.h>
#include <iostream>
typedef int (*NoNameFunc)();
extern "C" _declspec(dllimport) int sum1(int a, int b);


