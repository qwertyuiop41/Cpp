#include<Windows.h>

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
	// 2. 申请一段内存空间，用于存放dll name（便于后续调用LoadLibraryA函数加载DLL）
	// 要么存放绝对路径
	// 要么存放文件名（相对路径），如果是文件名则要吧dll文件放在游戏进程的工作目录下，因为这是在游戏进程下执行的，LoadLibraryA会在游戏目录下查找dll文件。
	char dllname[] = "Sword2MFC.dll";
	int dllnameLen = strlen(dllname);
	LPVOID pMemory = VirtualAllocEx(hProcess, NULL, dllnameLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (pMemory==NULL)
	{
		MessageBoxA(NULL, "申请内存失败!", "提示", MB_OK | MB_ICONERROR);
		return 0;
	}

	// 3. 将dll name写入申请的内存空间
	SIZE_T realWrite = 0;
	
	BOOL success= WriteProcessMemory(hProcess, pMemory, (LPCVOID)dllname, dllnameLen, &realWrite);
	if (success==FALSE)
	{
		MessageBoxA(NULL, "写入内存失败!", "提示", MB_OK | MB_ICONERROR);
		return 0;
	}

	// 4. 创建远程线程执行自定义函数
	DWORD remotePid;
	// dll文件名作为参数传给LoadLibraryA函数
	HANDLE remoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pMemory, 0, &remotePid);
	if (remoteThread == NULL)
	{
		MessageBoxA(NULL, "创建内存失败!", "提示", MB_OK | MB_ICONERROR);
		return 0;
	}

	// 5. 扫尾。释放内存空间
	// 等待线程执行完毕
	WaitForSingleObject(remoteThread, -1);
	VirtualFreeEx(hProcess, pMemory, 0, MEM_RELEASE);
	CloseHandle(remoteThread);
	CloseHandle(hProcess);

	system("pause");



	return 0;
}