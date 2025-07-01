#pragma once
#include <windows.h>
class __declspec(dllexport) mydll {
public:
	static int sum(int a, int b);
	static int div(int a, int b);
	static void fun1(char a, char c);
};

