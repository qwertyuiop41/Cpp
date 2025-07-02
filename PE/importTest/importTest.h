#pragma once
#pragma comment(lib ,"mydll3.lib")
extern "C" _declspec(dllimport) int sum1(int a, int b);
extern "C" _declspec(dllimport) void fun(char a, char c);
extern "C" _declspec(dllimport) int fun2();
extern "C" _declspec(dllimport) int test11();
extern "C" _declspec(dllimport) int test22();

