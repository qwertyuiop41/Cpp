#include <Windows.h>  
#include <cstdio> // Add this header for printf  

int main() {  
	DWORD funStart = 0x0041FD40;
	DWORD funReturn = 0x0041FD48;
	//char originFunStart[8] = {
	//	0x56,             // push esi
	//	0x8B, 0xF1,       // mov esi, ecx
	//	0x57,             // push edi
	//	0x83, 0x7E, 0x04, 0x02  // cmp dword ptr [esi+4], 2
	//};
	char hookCode[8] = {
		0xE9, 0xBB, 0x02, 0x31, 0x00, // jmp 0x00730000�������������ַ��Ҫ�滻��
		0x90,                         // nop
		0x90,                         // nop
		0x90                          // nop
	};
	char code[]= {
		0x56,                               // push esi
		0x8B, 0xF1,                         // mov esi, ecx
		0x57,                               // push edi
		0x81, 0x7E, 0x04, 0x02, 0x00, 0x00, 0x00,  // cmp [esi+04], 0x00000002
		0x75, 0x08,                        // jne 0x00730015 (�����ת 8�ֽ�)
		0xC7, 0x44, 0x24, 0x0C, 0x00, 0x00, 0x00, 0x00, // mov [esp+0C], 0x00000000
		0xE9, 0x2E, 0xFD, 0xCE, 0xFF       // jmp Sword2.exe+1FD48�������������ַҲҪ�������¼�������ƫ��
	};

	// 1. �ҵ���Ϸ���̲�����Ϸ����
	HWND hwnd = FindWindowA(NULL, "Sword2 Window"); // �滻Ϊʵ����Ϸ��������
	DWORD pid;
	if(hwnd == NULL) {
		MessageBoxA(NULL, "δ�ҵ���Ϸ����!", "��ʾ", MB_OK | MB_ICONERROR);
		return 0;
	}
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid==0)
	{
		MessageBoxA(NULL, "δ�ҵ���Ϸ����!", "��ʾ", MB_OK | MB_ICONERROR);
		return 0;
	}
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL) {
		MessageBoxA(NULL, "�޷�����Ϸ����!", "��ʾ", MB_OK | MB_ICONERROR);
		return 0;
	}
	// 2. ����һ���ڴ�ռ�
	char* pMemory = (char*)VirtualAllocEx(hProcess, NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// 3. �ҵ����������������޸ģ��޸�ǰ8���ֽڣ�jmp���޵��߼���
	DWORD oldProtect;
	SIZE_T realWrite = 0;
	DWORD offset = (DWORD)pMemory-funStart - 5; // ������תƫ����
	memcpy(hookCode+1, &offset, 4); // ��ƫ�������µ�ַ��д��hookCode
	VirtualProtectEx(hProcess, (LPVOID)funStart, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
	WriteProcessMemory(hProcess, (LPVOID)funStart, hookCode, sizeof(hookCode), &realWrite);
	VirtualProtectEx(hProcess, (LPVOID)funStart, 8, oldProtect, &oldProtect);

	// 4. ��������ڴ�ռ���д���޵��߼�
	DWORD offset2 = funReturn - (DWORD)pMemory - sizeof(code); // ������תƫ����
	SIZE_T realWrite2 = 0;
	memcpy(code + 22, &offset2, 4); // ��ƫ�������µ�ַ��д��code
	WriteProcessMemory(hProcess, (char*)pMemory, code, sizeof(code), &realWrite2);
	system("pause"); // ��ͣ���ȴ�����
	return 0;  
}