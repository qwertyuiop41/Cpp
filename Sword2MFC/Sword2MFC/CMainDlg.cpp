// CMainDlg.cpp: 实现文件
//

#include "pch.h"
#include "Sword2MFC.h"
#include "afxdialogex.h"
#include "CMainDlg.h"
#include "GameUtil.h"


// CMainDlg 对话框
IMPLEMENT_DYNAMIC(CMainDlg, CDialogEx)
GameUtil g_gameUtil;

CMainDlg::CMainDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_invincible(FALSE)
{

}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, m_invincible);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK1, &CMainDlg::OnBnClickedCheck1)
END_MESSAGE_MAP()


// CMainDlg 消息处理程序

void CMainDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL result = FALSE;
	UpdateData(TRUE); // 更新数据
	if (m_invincible==1)
	{
		result=g_gameUtil.setInvincibility();
		MessageBoxA(0, "设置无敌！", "提示", MB_OK | MB_ICONERROR);
	}
	else {
		result=g_gameUtil.closeInvincibility();
		MessageBoxA(0, "取消无敌！", "提示", MB_OK | MB_ICONERROR);
	}
	if (result==FALSE)
	{
		MessageBoxA(0, "操作失败！", "提示", MB_OK | MB_ICONERROR);
	}

}
