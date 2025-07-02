#include<Windows.h>
#include<iostream>
#include "PeUtil.h"
int main() {
	PeUtil peUtil;
	BOOL bSuccess= peUtil.LoadFile("importTest.exe");
	if (bSuccess)
	{
		peUtil.PrintSectionHeaders();
		//peUtil.GetExportTable();
		peUtil.GetImportTable();
		return 0;
	}
	printf("Failed to load the PE file.\n");
	return 0;
}