// Sword2MFC.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "Sword2MFC.h"
#include "CMainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CSword2MFCApp, CWinApp)
END_MESSAGE_MAP()

CMainDlg* pMainDlg{};


// CSword2MFCApp 构造
CSword2MFCApp::CSword2MFCApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CSword2MFCApp 对象
CSword2MFCApp theApp;


DWORD WINAPI ShowWindow(LPVOID lpParam)
{
	//调用 DoModal() 以模态方式显示对话框。模态对话框会阻塞程序流程，直到用户关闭它（如点击“确定”或“取消”），然后才会继续执行后续代码。
	pMainDlg->DoModal();
	delete pMainDlg;
	return 0;
}

// CSword2MFCApp 初始化
BOOL CSword2MFCApp::InitInstance()
{
	CWinApp::InitInstance();
	pMainDlg = new CMainDlg;
	this->m_pMainWnd = pMainDlg;
	
	// 防止阻塞，所以另外创建一个线程来显示对话框。
	::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowWindow, 0, 0, NULL);
	// 用户关闭对话框后，对话框对象不再需要，因此手动释放内存以防止内存泄漏。
	
	return TRUE;
}
