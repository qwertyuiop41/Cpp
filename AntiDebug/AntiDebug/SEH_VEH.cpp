//#include <Windows.h>
//#include <iostream>
//
//
////int main() {
////
////	__try {
////		char* str = NULL;
////		str[0] = 'a';
////	}
////	__except(1){
////		printf("Exception!\n");
////	}
////
////	printf("hello world\n");
////	system("pause");
////	return 0;
////}
//
//EXCEPTION_DISPOSITION  myExceptHandler(
//	struct _EXCEPTION_RECORD* ExceptionRecord,
//	PVOID EstablisherFrame,
//	PCONTEXT pcontext, //�Ĵ�������
//	PVOID DispatcherContext
//)
//{
//	MessageBoxA(0, "SEH�쳣�������ִ���ˣ�", "��ʾ", MB_OK);
//
//	DWORD isDebug = 0;
//	__asm {
//		mov eax,fs:[0x18] //teb
//		mov eax,[eax+0x30] //peb
//		movzx eax, byte ptr[eax + 0x2] //peb->BeingDebugged
//		mov isDebug, eax //������洢��isDebug��
//	}
//	printf("%d\n", isDebug);
//	if (isDebug)
//	{
//		MessageBoxA(0, "���ڱ�����", "��ʾ", MB_OK);
//		exit(0);
//	}
//
//	pcontext->Eip = pcontext->Eip + 3;
//
//	return ExceptionContinueExecution;
//}
//
//PVECTORED_EXCEPTION_HANDLER PvectoredExceptionHandler;
//
//LONG WINAPI PvectoredExceptionHandler(
//	 _EXCEPTION_POINTERS* ExceptionInfo
//)
//{
//	MessageBoxA(0, "VEH�쳣�������ִ���ˣ�", "��ʾ", MB_OK);
//	//ExceptionInfo->ContextRecord->Eip = ExceptionInfo->ContextRecord->Eip + 3;
//	//return EXCEPTION_CONTINUE_EXECUTION;
//	return EXCEPTION_CONTINUE_SEARCH;
//}
//
//int main() {
//	AddVectoredExceptionHandler(1, PvectoredExceptionHandler);
//
//	DWORD exceptionHandler = (DWORD)myExceptHandler;
//	__asm {
//		push exceptionHandler
//		mov eax, fs:[0]
//		push eax
//		mov fs:[0],esp 
//	}
//
//	char* str = NULL;
//	str[0] = 'a';
//
//	printf("hello world\n");
//	system("pause");
//	return 0;
//}