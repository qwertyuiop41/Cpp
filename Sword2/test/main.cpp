

#include <stdio.h>
#include<Windows.h>

const char* s1 = "Hell", * s2 = "pause";
int main() {
	_asm {
		push 0x00000052
		call dword ptr[printf]

	}

	return 0;
}
