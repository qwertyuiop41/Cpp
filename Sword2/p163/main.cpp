#include<Windows.h>




DWORD _stdcall AutoAttack(LPVOID lparam) {
	DWORD* hp = (DWORD*)0x004CEF18;
	while (true){
		if (*hp<=500)
		{
			//push 0x00000052
			//mov ecx, 00537400
			//call 004252E0
			//��������������������ģ�������C/C++ inline assembly�в���ֱ��call��ַ��
			// ������Ҫ�ѵ�ַ����Ĵ�����call�Ĵ���
			_asm 
			{
				push 0x00000052
				mov ecx, 0x00537400
				mov eax, 0x004252E0
				call eax
			}
		}
	}
	return 0;

}

int main() {

	// 1. �ҵ���Ϸ���̲�����Ϸ����
	HWND hwnd = FindWindowA(NULL, "Sword2 Window"); // �滻Ϊʵ����Ϸ��������
	DWORD pid;
	if (hwnd == NULL) {
		MessageBoxA(NULL, "δ�ҵ���Ϸ����!", "��ʾ", MB_OK | MB_ICONERROR);
		return 0;
	}
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == 0)
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
	LPVOID pMemory = VirtualAllocEx(hProcess, NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	
	// 3. ���Զ��庯��д��������ڴ�ռ�
	SIZE_T realWrite = 0;
	WriteProcessMemory(hProcess, pMemory, (LPCVOID)AutoAttack, 1024, &realWrite);

	// 4. ����Զ���߳�ִ���Զ��庯��
	DWORD remotePid;
	HANDLE remoteThread= CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pMemory, NULL, 0, &remotePid);

	system("pause");



	return 0;
}