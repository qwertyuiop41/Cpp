#include<Windows.h>

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
	// 2. ����һ���ڴ�ռ䣬���ڴ��dll name�����ں�������LoadLibraryA��������DLL��
	// Ҫô��ž���·��
	// Ҫô����ļ��������·������������ļ�����Ҫ��dll�ļ�������Ϸ���̵Ĺ���Ŀ¼�£���Ϊ��������Ϸ������ִ�еģ�LoadLibraryA������ϷĿ¼�²���dll�ļ���
	char dllname[] = "Sword2MFC.dll";
	int dllnameLen = strlen(dllname);
	LPVOID pMemory = VirtualAllocEx(hProcess, NULL, dllnameLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (pMemory==NULL)
	{
		MessageBoxA(NULL, "�����ڴ�ʧ��!", "��ʾ", MB_OK | MB_ICONERROR);
		return 0;
	}

	// 3. ��dll nameд��������ڴ�ռ�
	SIZE_T realWrite = 0;
	
	BOOL success= WriteProcessMemory(hProcess, pMemory, (LPCVOID)dllname, dllnameLen, &realWrite);
	if (success==FALSE)
	{
		MessageBoxA(NULL, "д���ڴ�ʧ��!", "��ʾ", MB_OK | MB_ICONERROR);
		return 0;
	}

	// 4. ����Զ���߳�ִ���Զ��庯��
	DWORD remotePid;
	// dll�ļ�����Ϊ��������LoadLibraryA����
	HANDLE remoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pMemory, 0, &remotePid);
	if (remoteThread == NULL)
	{
		MessageBoxA(NULL, "�����ڴ�ʧ��!", "��ʾ", MB_OK | MB_ICONERROR);
		return 0;
	}

	// 5. ɨβ���ͷ��ڴ�ռ�
	// �ȴ��߳�ִ�����
	WaitForSingleObject(remoteThread, -1);
	VirtualFreeEx(hProcess, pMemory, 0, MEM_RELEASE);
	CloseHandle(remoteThread);
	CloseHandle(hProcess);

	system("pause");



	return 0;
}