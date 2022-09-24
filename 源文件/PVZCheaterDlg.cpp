// PVZCheaterDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "PVZCheater.h"
#include "PVZCheaterDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static CPVZCheaterDlg* g_dlg;
static HANDLE g_processHandle;

// 将某个值写入植物大战僵尸内存（后面的可变参数是地址链，要以-1结尾）
void WriteMemory(void* value, DWORD valueSize, ...)
{
	if (value == NULL || valueSize == 0 || g_processHandle == NULL) return;

	DWORD tempValue = 0;

	va_list addresses;
	va_start(addresses, valueSize);
	DWORD offset = 0;
	DWORD lastAddress = 0;
	while ((offset = va_arg(addresses, DWORD)) != -1)
	{
		lastAddress = tempValue + offset;
		::ReadProcessMemory(g_processHandle, (LPCVOID)lastAddress, &tempValue, sizeof(DWORD), NULL);
	}
	va_end(addresses);

	::WriteProcessMemory(g_processHandle, (LPVOID)lastAddress, value, valueSize, NULL);
}

void WriteMemory(void* value, DWORD valueSize, DWORD address) //重载
{
	WriteMemory(value, valueSize, address, -1);
}

//用来监控游戏的线程

DWORD MonitorThreadFunc(LPVOID lpThreadParameter)
{
	//MainWindow
	//植物大战僵尸中文版
	while (true)
	{
		//获得植物大战僵尸窗口句柄
		HWND handle = FindWindow(CString("MainWindow"), CString("植物大战僵尸中文版"));

		if (handle == NULL)
		{
			g_dlg->m_bnKill.SetCheck(FALSE);
			g_dlg->m_bnSun.SetCheck(FALSE);

			g_dlg->m_bnKill.EnableWindow(FALSE);
			g_dlg->m_bnSun.EnableWindow(FALSE);

			g_processHandle = NULL;
		}
		else if (g_processHandle == NULL)
		{
			g_dlg->m_bnKill.EnableWindow(true);
			g_dlg->m_bnSun.EnableWindow(true);

			//获得植物大战僵尸的进程ID
			DWORD pid;
			GetWindowThreadProcessId(handle, &pid);
			//找PVZ进程句柄
			g_processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		}

		if (g_dlg->m_bnSun.GetCheck()) //无限阳光
		{
			DWORD value = 9999;
			WriteMemory(&value, sizeof(value), 0x6A9EC0, 0x320, 0x8, 0x0, 0x8, 0x144, 0x2c, 0x5560, -1);
		}

		Sleep(1000);
	}
	return NULL;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CPVZCheaterDlg 对话框

CPVZCheaterDlg::CPVZCheaterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PVZCHEATER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPVZCheaterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KILL, m_bnKill);
	DDX_Control(pDX, IDC_SUN, m_bnSun);
}

BEGIN_MESSAGE_MAP(CPVZCheaterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_TEST, CPVZCheaterDlg::OnButtonChlickenTest)
	ON_BN_CLICKED(IDC_KILL, &CPVZCheaterDlg::OnBnClickedKill)
	ON_BN_CLICKED(IDC_SUN, CPVZCheaterDlg::OnBnClickedSun)
END_MESSAGE_MAP()

// CPVZCheaterDlg 消息处理程序

BOOL CPVZCheaterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//创建子线程
	m_GameMonitorThread = CreateThread(NULL, NULL, MonitorThreadFunc, NULL, NULL, NULL);

	//保存对话框
	g_dlg = this;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPVZCheaterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPVZCheaterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPVZCheaterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/*int a = 10;
CString str;
str.Format(CString("test %d"), a);
AfxMessageBox(str);
输出方式*/

void CPVZCheaterDlg::OnButtonChlickenTest()
{
	ShellExecute(NULL, CString("open"),
		CString("https://www.baidu.com/"),
		NULL, NULL,
		SW_SHOWNORMAL);
	/*
	加按钮
	改按钮ID   在 BEGIN_MESSAGE_MAP
	绑定ID
	头文件加函数，写函数
	*/
}

//BOOL checked = IsDlgButtonChecked(IDC_KILL);
//if (checked)
//{
//
//}
//else
//{
//
//}
//判断是否勾选check

//CButton* button = (CButton*)GetDlgItem(IDC_KILL);
	//if (button->GetCheck())
	//{
	//	//勾选
	//}else
	//{
	//	//没有勾选
	//}

void CPVZCheaterDlg::OnBnClickedKill()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bnKill.GetCheck())
	{
		BYTE data[] = { 0xFF,0x90,0x9090 };
		WriteMemory(data, sizeof(data), 0x00531310);
	}
	else
	{
		BYTE data[] = { 0x7c,0x24,0x20 };
		WriteMemory(data, sizeof(data), 0x00531310);
	}
}

void CPVZCheaterDlg::OnBnClickedSun()
{
	// TODO: 在此添加控件通知处理程序代码
}