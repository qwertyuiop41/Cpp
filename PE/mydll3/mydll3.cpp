#include "mydll3.h"


// ͷ�ļ��������ĺ���ʵ�֣�����ԭ����
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



// DEF�ļ��е����ĺ�����Ҫ���extern "C"����
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

// ����
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
