#include<Windows.h>
#include<iostream>
#include "CPeUtil.h"
int main() {
	CPeUtil peUtil;
	BOOL bSuccess = peUtil.LoadFile("importTest.exe");
	BOOL bInsert = peUtil.InsertSection();
	return 0;
}