#include<Windows.h>




DWORD _stdcall AutoAttack(LPVOID lparam) {
	DWORD* hp = (DWORD*)0x004CEF18;
	while (true){
		if (*hp<=500)
		{
			//push 0x00000052
			//mov ecx, 00537400
			//call 004252E0
			//本来汇编码是上面这样的，但是在C/C++ inline assembly中不能直接call地址，
			// 所以需要把地址存入寄存器再call寄存器
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

	// 1. 找到游戏进程并打开游戏进程
	HWND hwnd = FindWindowA(NULL, "Sword2 Window"); // 替换为实际游戏窗口名称
	DWORD pid;
	if (hwnd == NULL) {
		MessageBoxA(NULL, "未找到游戏窗口!", "提示", MB_OK | MB_ICONERROR);
		return 0;
	}
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == 0)
	{
		MessageBoxA(NULL, "未找到游戏进程!", "提示", MB_OK | MB_ICONERROR);
		return 0;
	}
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL) {
		MessageBoxA(NULL, "无法打开游戏进程!", "提示", MB_OK | MB_ICONERROR);
		return 0;
	}
	// 2. 申请一段内存空间
	LPVOID pMemory = VirtualAllocEx(hProcess, NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	
	// 3. 将自定义函数写入申请的内存空间
	SIZE_T realWrite = 0;
	WriteProcessMemory(hProcess, pMemory, (LPCVOID)AutoAttack, 1024, &realWrite);

	// 4. 创建远程线程执行自定义函数
	DWORD remotePid;
	HANDLE remoteThread= CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pMemory, NULL, 0, &remotePid);

	system("pause");



	return 0;
}