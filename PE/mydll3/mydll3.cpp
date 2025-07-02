#include "mydll3.h"


// 头文件中声明的函数实现（保持原样）
int sum1(int a, int b)
{
    return a+b;
}


 int test11() {
    return 11;
}

int test22() {
    return 22;
}
int fun2() {
    return 2;
}



// DEF文件中导出的函数需要添加extern "C"声明
//extern "C" {
//    int fun2()
//    {
//        return 1;
//    }
//
//    int test11()
//    {
//        return 11;
//    }
//
//    int test22()
//    {
//        return 22;
//    }
//}

// 或者
//extern "C" int fun2()
//{
//    return 1;
//}
//
//extern "C" int test11()
//{
//    return 11;
//}
//
//extern "C" int test22()
//{
//    return 22;
//}

// mydll3.cpp
