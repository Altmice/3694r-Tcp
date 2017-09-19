
// ClientTCPDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <time.h>
#include <iostream>
#include "ClientTCP.h"
#include "ClientTCPDlg.h"
#include "afxdialogex.h"
#include "platform.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//定义TCP 客户端接收到数据消息
#define WM_RECV_TCP_DATA WM_USER + 101
//定义TCP客户端连接断开消息
#define WM_TCP_CLIENT_DISCONNECT WM_USER + 102
// CClientTCPDlg 对话框

const int CClientTCPDlg::LogStatus=1;
const int CClientTCPDlg::RetryStatus=1;


CClientTCPDlg::CClientTCPDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientTCPDlg::IDD, pParent)
	, m_remotePort(0)
	, m_remoteHost(_T(""))
{
	Peroid=1;
	HandShakeStatus=0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientTCPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_HOST, m_remotePort);
	DDX_Text(pDX, IDC_EDIT_PORT, m_remoteHost);
}

BEGIN_MESSAGE_MAP(CClientTCPDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_RECV_TCP_DATA,OnRecvTCPData)
	ON_MESSAGE(WM_TCP_CLIENT_DISCONNECT,OnClientDisconnect)
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CClientTCPDlg::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_BTN_DISCONNECT, &CClientTCPDlg::OnBnClickedBtnDisconnect)
	ON_BN_CLICKED(IDC_BTN_SENDDATA, &CClientTCPDlg::OnBnClickedBtnSenddata)
	ON_BN_CLICKED(IDC_BTN_SET, &CClientTCPDlg::OnBnClickedBtnSet)
	//ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON2, &CClientTCPDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_CHECK3964R, &CClientTCPDlg::OnBnClickedCheck3964r)
END_MESSAGE_MAP()


// CClientTCPDlg 消息处理程序

BOOL CClientTCPDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_remoteHost=GetLocalIP();
	m_remotePort=23;
	UpdateData(FALSE);
	CButton *Button=(CButton*)GetDlgItem(IDC_BTN_DISCONNECT);
	Button->EnableWindow(false);
	Button=(CButton*)GetDlgItem(IDC_BTN_SENDDATA);
	Button->EnableWindow(false);
	CButton *CheckBox=(CButton*)GetDlgItem(IDC_CHECK3964R);
	CheckBox->SetCheck(1);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientTCPDlg::OnPaint()
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
HCURSOR CClientTCPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//连接断开事件
void CALLBACK CClientTCPDlg::OnDisConnect(void* pOwner)
{
	//得到父对象指针
	CClientTCPDlg* pThis = (CClientTCPDlg*)pOwner;
	//发送消息表示客户端连接断开
	pThis->PostMessage(WM_TCP_CLIENT_DISCONNECT,0,0);
}

//数据接收事件
void CALLBACK CClientTCPDlg::OnRead(void* pOwner,const char * buf,DWORD dwBufLen )
{
	BYTE *pRecvBuf = NULL; //接收缓冲区
	//得到父对象指针
	CClientTCPDlg* pThis = (CClientTCPDlg*)pOwner;
	//将接收的缓冲区拷贝到pRecvBuf种
	pRecvBuf = new BYTE[dwBufLen];
	CopyMemory(pRecvBuf,buf,dwBufLen);

	//发送异步消息，表示收到TCP数据，消息处理完，应释放内存
	pThis->PostMessage(WM_RECV_TCP_DATA,WPARAM(pRecvBuf),dwBufLen);

}

//Socket错误事件
void CALLBACK CClientTCPDlg::OnError(void* pOwner,int nErrorCode)
{
	TRACE(L"客户端socket发生错误");
}

//TCP接收数据处理函数
//Peroid 0发 2收 1空置
//HandShakeStatus 发状态1是握手正常 收状态11和13是两次握手正常
LONG CClientTCPDlg::OnRecvTCPData(WPARAM wParam,LPARAM lParam)
{
	CString strOldRecv = L"";//老对话框字符 不用了
	CString strRecv = L"";//用于CString的处理方式
	//接收到的BUF
	CHAR *pBuf = (CHAR*)wParam;
	//接收到的BUF长度
	DWORD dwBufLen = lParam;
	//接收框
	//CEdit *pEdtRecvMsg = (CEdit*)GetDlgItem(IDC_EDIT_RECEIVE);
	//ASSERT(pEdtRecvMsg != NULL);

	if (m_tcpClient.Status3964R)
	{
		char tmp;
		switch (Peroid)
		{
			case 0://发送
				
				tmp=0;
				tmp=pBuf[0];
				switch (tmp)
				{
					case DLE://正常数据
						HandShakeStatus=1;
						if (LogStatus)
						{
							PrintHexChar(pBuf,1,RECVSTATUS);
						}
						break;
					case NAK://NAK崩
						HandShakeStatus=-1;
						if (LogStatus) 
						{
							PrintHexChar(pBuf,1,RECVSTATUS);
						}
						break;
					default:
						break;
				}
				break;
			case 1:
				break;
			case 2://监听
			
				tmp=0;
				tmp=pBuf[0];
				char CtrlString[4];
				CtrlString[0]=DLE;CtrlString[1]=NULL;
				switch (tmp)
				{
					case STX:

						if (!m_tcpClient.SendData(CtrlString,1))
						{
							AfxMessageBox(_T("回复STX失败"));
							if (LogStatus)
							{
								fprintf_s(fErr,"Send:回复STX失败\n");
							}
							Peroid=1;
							return 0;
						}
						if (LogStatus)
						{
							PrintHexChar(pBuf,1,RECVSTATUS);
							PrintHexChar(CtrlString,1,SENDSTATUS);
							fprintf_s(fErr,"收机械手握手1完成\n");
						}
						HandShakeStatus=11;//握手1Ok
						SetTimer(TIMERRECV,WAITRECV,NULL);//设置响应时间
						break;
					case NAK:
						AfxMessageBox(_T("通信NAK"));
						if (LogStatus)
						{
							fprintf_s(fErr,"Recv:接收通信NAK\n");
							PrintHexChar(pBuf,1,RECVSTATUS);
						}
						HandShakeStatus=-1;
						break;
					
					default:
						if (HandShakeStatus==11)
						{
							if (!m_tcpClient.SendData(CtrlString,1))//握手2完成
							{
								AfxMessageBox(_T("回复DataString失败"));
								if (LogStatus)
								{
									fprintf_s(fErr,"Send:回复DataString失败\n");
								}
								Peroid=1;
								break;
							}
							KillTimer(TIMERRECV);//关等数据计时
							HandShakeStatus=13;
							strcpy(RecvAStr,pBuf);//取数据
							RecvAStr[7] = 0x00;
							if (LogStatus)
							{
								PrintHexChar(RecvAStr,strlen(RecvAStr),RECVSTATUS);
								PrintHexChar(CtrlString,1,SENDSTATUS);
								fprintf_s(fErr,"收机械手握手2完成\n");
							}
							delete[] pBuf;
							
							pBuf = NULL;
						}
						
						break;
				}
				break;
		}
				
	}
	else
	{
		////得到接收框中的历史文本
		//pEdtRecvMsg->GetWindowTextW(strOldRecv);
		//
		int len=MultiByteToWideChar(CP_ACP,0,LPCSTR(pBuf),dwBufLen,NULL,0);
		wchar_t* pUnicode;
		pUnicode=new wchar_t[len+1];
		memset(pUnicode,0,(len+1)*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP,0,LPCSTR(pBuf),dwBufLen,pUnicode,len);
		strRecv.Empty();strRecv.Append(pUnicode);

		//将新接收到的文本添加到接收框中
		strOldRecv.Append(pUnicode);
		int pos=strRecv.Find(_T(":"));
		if (pos!=-1)
		{
			if (strRecv.Mid(pos+1,1)==_T("E"))
			{
				DMeasureResult=-1;
			}
			else
			{
				if (strRecv.Find(_T("m"))>0)
				{
					DMeasureResult=1;
					fMResult=_wtof(strRecv.Mid(pos+1,strRecv.Find(_T("m"))-pos-1));
					pos=0;
				}	
			}
		}
	
		//strOldRecv = strOldRecv + L"\r\n";
		//pEdtRecvMsg->SetWindowTextW(strOldRecv);

		//释放内存
		delete[] pBuf;
		delete[] pUnicode;
		pBuf = NULL;
	}
	
	return 0;
}

//客户端连接断开消息函数
LONG CClientTCPDlg::OnClientDisconnect(WPARAM wParam,LPARAM lParam)
{
	//得到状态栏标签
	CStatic * pStatus = (CStatic *)GetDlgItem(IDC_CONNECTSTATUS);
	ASSERT(pStatus != NULL);

	pStatus->SetWindowText(_T("连接断开"));
	return 0;
}

//得到本地的IP地址
CString CClientTCPDlg::GetLocalIP()
{
	HOSTENT *LocalAddress;
	char	*Buff;
	TCHAR	*wBuff;
	CString strReturn = _T("");

	//创建新的缓冲区
	Buff = new char[256];
	wBuff = new TCHAR[256];
	//置空缓冲区
	memset(Buff, '\0', 256);
	memset(wBuff, TEXT('\0'), 256*sizeof(TCHAR));
	//得到本地计算机名
	if (gethostname(Buff, 256) == 0)
	{
		//转换成双字节字符串
		mbstowcs(wBuff, Buff, 256);
		//得到本地地址
		LocalAddress = gethostbyname(Buff);
		//置空buff
		memset(Buff, '\0', 256);
		//组合本地IP地址
		sprintf(Buff, "%d.%d.%d.%d\0", LocalAddress->h_addr_list[0][0] & 0xFF,
			LocalAddress->h_addr_list[0][1] & 0x00FF, LocalAddress->h_addr_list[0][2] & 0x0000FF, LocalAddress->h_addr_list[0][3] & 0x000000FF);
		//置空wBuff
		memset(wBuff, TEXT('\0'), 256*sizeof(TCHAR));
		//转换成双字节字符串
		mbstowcs(wBuff, Buff, 256);
		//设置返回值
		strReturn = wBuff;
	}
	else
	{
	}

	//释放Buff缓冲区
	delete[] Buff;
	Buff = NULL;
	//释放wBuff缓冲区
	delete[] wBuff;
	wBuff = NULL;
	return strReturn;
}


void CClientTCPDlg::OnBnClickedBtnConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CStatic *pStatus = (CStatic*)GetDlgItem(IDC_CONNECTSTATUS);
	ASSERT(pStatus != NULL);
	//设置m_tcpClient属性
	m_tcpClient.m_remoteHost = m_remoteHost;
	m_tcpClient.m_port = m_remotePort;
	m_tcpClient.OnDisConnect = OnDisConnect;
	m_tcpClient.OnRead = OnRead;
	m_tcpClient.OnError = OnError;
	//打开客户端socket
	m_tcpClient.Open(this);

	//建立与服务器端连接
	if (m_tcpClient.Connect())
	{

		pStatus->SetWindowText(L"建立连接");
		UpdateData(FALSE);
	}
	else
	{
		AfxMessageBox(_T("建立连接失败"));
		pStatus->SetWindowText(L"连接断开");
		return;
	}
	CButton *Button=(CButton*)GetDlgItem(IDC_BTN_CONNECT);
	Button->EnableWindow(false);
	Button=(CButton*)GetDlgItem(IDC_BTN_DISCONNECT);
	Button->EnableWindow(true);
	Button=(CButton*)GetDlgItem(IDC_BTN_SENDDATA);
	Button->EnableWindow(true);
}



void CClientTCPDlg::OnBnClickedBtnDisconnect()
{
	// TODO: 在此添加控件通知处理程序代码
	CStatic *pStatus = (CStatic*)GetDlgItem(IDC_CONNECTSTATUS);
	ASSERT(pStatus != NULL);
	//关闭客户端套接字
	if (m_tcpClient.Close())
	{
		pStatus->SetWindowText(L"连接断开");
	}
	else
	{
		AfxMessageBox(_T("连接断开失败"));
	}	
	CButton *Button=(CButton*)GetDlgItem(IDC_BTN_CONNECT);
	Button->EnableWindow(true);
	Button=(CButton*)GetDlgItem(IDC_BTN_DISCONNECT);
	Button->EnableWindow(false);
	Button=(CButton*)GetDlgItem(IDC_BTN_SENDDATA);
	Button->EnableWindow(false);
}


void CClientTCPDlg::OnBnClickedBtnSenddata()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strSendData;
	char * sendBuf;
	int sendLen=0;
	CEdit *pEdtSend = (CEdit*)GetDlgItem(IDC_EDIT_SEND);
	pEdtSend->GetWindowTextW(strSendData);

	//设置发送缓冲区
//	sendLen = strSendData.GetLength()*2 + 2;
//	sendBuf = new char[sendLen];
//	ZeroMemory(sendBuf,sendLen);
//	wcstombs(sendBuf,strSendData,sendLen);
	
	//将待发送的字符串转换成单字节，进行发送
	//sendBuf = new char[strSendData.GetLength()*2+1];
	//ZeroMemory(sendBuf,strSendData.GetLength()*2+1);
	//转换成单字节进行发送	
	//WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,strSendData.GetBuffer(strSendData.GetLength())
	//   ,strSendData.GetLength(),sendBuf,strSendData.GetLength()*2,NULL,NULL);
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK3964R))
	{
		char CtrlString[4];
		MSG msg;
		char Bcc=0;
		DWORD dBufSize=WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, NULL,0,NULL, FALSE);  
		//分配目标缓存  
		sendBuf = new char[dBufSize+3];  
		memset(sendBuf, 0, dBufSize+3);  
		//转换  
		WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, sendBuf, dBufSize, NULL, FALSE);
		//sendBuf
		//sendBuf[dBufSize-1]=0x30;sendBuf[dBufSize]=0x31;
		sendBuf[dBufSize-1]=DLE;sendBuf[dBufSize]=ETX;
		Bcc=sendBuf[0];
		for (int i=1;i<dBufSize+2;i++)
		{
			Bcc=Bcc^sendBuf[i];
		}
		sendBuf[dBufSize+1]=Bcc;
		sendLen = strlen(sendBuf)+1;
		CtrlString[0]=STX;CtrlString[1]=NULL;
		Peroid=0;HandShakeStatus=0;
		if (!m_tcpClient.SendData(CtrlString,1))
		{
			AfxMessageBox(_T("发送失败"));
			Peroid=1;
			return ;
		}
		SetTimer(TIMERSEND,WAITSEND,NULL);
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			switch (HandShakeStatus)
			{
				case -2:
					AfxMessageBox(_T("连接超时"));
					Peroid=1;
					KillTimer(TIMERSEND);
					return;
				case -1:
					AfxMessageBox(_T("握手Nak失败"));
					Peroid=1;
					KillTimer(TIMERSEND);
					return;
				case 0:
					break;
				case 1:
					KillTimer(TIMERSEND);
					break;
			}
			if (HandShakeStatus==1)
				break;
		}

		//DLE检查
		Peroid=0;HandShakeStatus=0;
		if (!m_tcpClient.SendData(sendBuf,sendLen))
		{
			AfxMessageBox(_T("发送失败"));
		}
		//释放内存
		delete[] sendBuf;
		sendBuf = NULL;		
		SetTimer(TIMERSEND,WAITSEND,NULL);
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			switch (HandShakeStatus)
			{
				case -2:
					AfxMessageBox(_T("连接超时"));
					Peroid=1;
					KillTimer(TIMERSEND);
					return;
				case -1:
					AfxMessageBox(_T("握手失败"));
					Peroid=1;
					KillTimer(TIMERSEND);
					return;
				case 0:
					break;
				case 1:
					AfxMessageBox(_T("发送完成"));
					Peroid=1;
					KillTimer(TIMERSEND);
					return ;
			}
		}	
	}
	else
	{
		DWORD dBufSize=WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, NULL,0,NULL, FALSE);  
		//分配目标缓存  
		sendBuf = new char[dBufSize];  
		memset(sendBuf, 0, dBufSize);  
	 //转换  
		WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, sendBuf, dBufSize, NULL, FALSE);
		sendLen = strlen(sendBuf)+1;
		DMeasureResult=0;
		//发送数据
		if (!m_tcpClient.SendData(sendBuf,sendLen))
		{
			AfxMessageBox(_T("发送失败"));
		}
		//释放内存
		delete[] sendBuf;
		sendBuf = NULL;		
		MSG msg;
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (DMeasureResult==1) 
			{
				CString str;
				str.Format(_T("测量成功%fm"),fMResult);
				AfxMessageBox(str);break;
			}
			if (DMeasureResult==-1) {AfxMessageBox(_T("测量失败"));break;}
		}
		AfxMessageBox(_T("测量完成"));
	}
	
}
int CClientTCPDlg::DistanceQuery(LPCTSTR Hostname,float* dist)
{

	m_tcpClient.m_remoteHost = Hostname;
	m_tcpClient.m_port = 23;
	m_tcpClient.OnDisConnect = OnDisConnect;
	m_tcpClient.OnRead = OnRead;
	m_tcpClient.OnError = OnError;
	for (int i=0;i<2;i++)
	{
		//打开客户端socket
		m_tcpClient.Open(this);

		//建立与服务器端连接
		if (m_tcpClient.Connect())
		{
			CString strSendData;
			char * sendBuf;
			int sendLen=0;
			strSendData.Format(_T("D"));
			DWORD dBufSize=WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, NULL,0,NULL, FALSE);  
			//分配目标缓存  
			sendBuf = new char[dBufSize];  
			memset(sendBuf, 0, dBufSize);  
		//转换  
			WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, sendBuf, dBufSize, NULL, FALSE);
			sendLen = strlen(sendBuf)+1;
			DMeasureResult=0;
			//发送数据
			if (!m_tcpClient.SendData(sendBuf,sendLen))
			{
				AfxMessageBox(_T("发送失败"));
				return -1;
			}
			//释放内存
			SetTimer(TIMERDIST,WAITDIST,NULL);
			delete[] sendBuf;
			sendBuf = NULL;		
			MSG msg;
			while (GetMessage(&msg,NULL,0,0L))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (DMeasureResult==1) 
				{
					CString str;
					str.Format(_T("测量成功%fm"),fMResult);
					*dist=fMResult;
					dist++;
					AfxMessageBox(str);break;
				}
				if (DMeasureResult==-1) {AfxMessageBox(_T("测量失败"));break;}
				if (DMeasureResult==-2) {AfxMessageBox(_T("测量超时，检查通信"));break;}
			}
			AfxMessageBox(_T("测量完成"));
		}
		else
		{
			AfxMessageBox(_T("建立连接失败"));
			return -1;
		}
		KillTimer(TIMERDIST);
		if (m_tcpClient.Close())
		{
		
		}
		else
		{
			AfxMessageBox(_T("连接断开失败"));
			return -1;
		}	
		m_tcpClient.m_port = 26;
		Sleep(WAITCONST);
	
	}
	
	
}
void  CClientTCPDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
		case TIMERDIST:
			DMeasureResult=-2;
			break;
		case TIMERSEND:
			HandShakeStatus=-2;
			break;
		case TIMERRECV:
			//AfxMessageBox(_T("收数据超时"));
			//KillTimer(TIMERRECV);
			HandShakeStatus=-2;
			break;
	}
}
void CClientTCPDlg::OnBnClickedBtnSet()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_remoteHost=GetLocalIP();
	//m_remotePort=23;
	UpdateData(FALSE);
}


void CClientTCPDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	//MSG msg;
	//while (GetMessage(&msg,NULL,0,0L))
	//{
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//	if (HandShakeStatus==13)
	//		break;
	//	else
	//	{
	//		if (HandShakeStatus==12)
	//			continue;
	//	}
	//	
	//}
	fErr=NULL;
	char dateStr[64];
	char fileName[64];
	GetSystemTime(dateStr);
	if (LogStatus)
	{
		sprintf_s(fileName,"Log\\%s.txt",dateStr);
		fErr=fopen(fileName,"at+");
		time_t t=time(NULL);
		strftime(dateStr,sizeof(dateStr),"%Y-%m-%d %X",localtime(&t));
		fprintf_s(fErr,"%s  换电流程开始\n",dateStr);
		
	}
	
	
	CString str=_T("192.168.0.2");
	int port=23;
	int num[5]={-500,200,1100,-800,500};
	bool flag;
	m_tcpClient.Status3964R=true;
	RemoteServerConnect(str,DWORD(port));
	//AfxMessageBox(L"RemoteConnected");
	//RecvOrder();
	//CString res(RecvAStr);
	//
	//AfxMessageBox(res);
	//Sleep(WAITCONST);
	//flag=SendNetStr("002");
	////Sleep(WAITCONST);
	//if (!flag&&(RetryStatus))
	//{
	//	int RetryTimes=RetryStatus;
	//	while (RetryTimes>0)
	//	{
	//		RetryTimes--;
	//		Sleep(5);
	//		flag=SendNetStr("002");
	//		if (flag)
	//			break;
	//	}
	//	if (!flag)
	//	{
	//		AfxMessageBox(_T("重试多次均不能发送"));
	//		if (LogStatus)
	//			fprintf_s(fErr,"重试多次均发送失败\n");
	//	}
	//}
	int cntt = 0;
	while (1)
	{
		RecvOrder();
		CString res1(RecvAStr);

		//AfxMessageBox(res1);
		Sleep(WAITCONST);
		flag = SendNetStr("+123 -224 833 4.33 3.22");
		if (!flag && (RetryStatus))
		{
			int RetryTimes = RetryStatus;
			while (RetryTimes > 0)
			{
				RetryTimes--;
				Sleep(5);
				flag = SendNetStr("+123 -224 833 4.33 3.22");
				if (flag)
					break;
			}
			if (!flag)
			{
				AfxMessageBox(_T("重试多次均不能发送"));
				if (LogStatus)
					fprintf_s(fErr, "重试多次均发送失败\n");
			}
		}
		else
		{
			cntt++;
			if (cntt>2) break;
		}
			
	}
	
		
	//SendNetNum(num,4);

	RemoteServerDisconnect();

	if (LogStatus)
	{
		time_t t = time(NULL);
		strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %X", localtime(&t));
		fprintf_s(fErr, "%s  换电流程结束\n", dateStr);
		fclose(fErr);
	}
}


void CClientTCPDlg::OnBnClickedCheck3964r()
{
	// TODO: 在此添加控件通知处理程序代码
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK3964R))
	{
		AfxMessageBox(_T("有标记"));
		m_tcpClient.Status3964R=true;
	}
	else
	{
		m_tcpClient.Status3964R=false;
		AfxMessageBox(_T("无标记"));
	}
}

//IP地址Hostname 端口号Portname
bool CClientTCPDlg::RemoteServerConnect(CString Hostname, DWORD Portname)
{
	//设置m_tcpClient属性
	m_tcpClient.m_remoteHost = Hostname;
	m_tcpClient.m_port = Portname;
	m_tcpClient.OnDisConnect = OnDisConnect;
	m_tcpClient.OnRead = OnRead;
	m_tcpClient.OnError = OnError;
	//打开客户端socket
	m_tcpClient.Open(this);

	//建立与服务器端连接
	if (m_tcpClient.Connect())
	{
		if (LogStatus)
			fprintf_s(fErr,"端口%d连接成功\n",Portname);
		return true;		
	}
	else
	{
		if (LogStatus)
			fprintf_s(fErr,"端口%d连接失败\n",Portname);
		AfxMessageBox(_T("建立连接失败"));
		return false;
	}

}

//数组头指针 发送的数字个数 其中每个数字占4个字节
bool CClientTCPDlg::SendNetNum(int* deltaDist, int numOfInt)
{
	if (m_tcpClient.Status3964R)
	{
		for (int k=0;k<numOfInt;k++)
		{
			char * sendBuf;
			int sendLen=0;
			char CtrlString[2];
			MSG msg;
			char Bcc=0;
			int dBufSize=4;
			//分配目标缓存  
			sendBuf = new char[dBufSize+4];  
			memset(sendBuf, 0, dBufSize+4);  
			//转换  
			sprintf(sendBuf,"%04d",abs(deltaDist[k]));
			sendBuf[dBufSize]=DLE;sendBuf[dBufSize+1]=ETX;
			if (abs(deltaDist[k])>1200)
			{
				AfxMessageBox(_T("数值计算有误"));
				Peroid=1;//静止状态
				return false;
			}
			else
			{
				if (sendBuf[0]==0x30)
				{
					if (deltaDist[k]>0)
					{
						sendBuf[0]=0x2B;
					}
					else
					{
						sendBuf[0]=0x2D;
					}
				}
			}
			Bcc=sendBuf[0];
			for (int i=1;i<dBufSize+2;i++)
			{
				Bcc=Bcc^sendBuf[i];
			}
			sendBuf[dBufSize+2]=Bcc;
			sendLen = strlen(sendBuf);
			CtrlString[0]=STX;CtrlString[1]=NULL;
			Peroid=0;HandShakeStatus=0;//发送态 握手IDLE
			if (!m_tcpClient.SendData(CtrlString,1))
			{
				AfxMessageBox(_T("发送失败"));
				Peroid=1;
				return false;
			}
			SetTimer(TIMERSEND,WAITSEND,NULL);//等0x10回复
			while (GetMessage(&msg,NULL,0,0L))//检索消息队列，等待处理recv_tcp_data
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				switch (HandShakeStatus)
				{
					case -2:
						AfxMessageBox(_T("连接超时"));//响应超时
						Peroid=1;//空置
						KillTimer(TIMERSEND);//关计时器
						return false;//返回错误
					case -1:
						AfxMessageBox(_T("握手Nak失败"));//0x15
						Peroid=1;//空置
						KillTimer(TIMERSEND);//关计时器
						return false;//错误
					case 0:
						break;//空置
					case 1:
						KillTimer(TIMERSEND);//0x10
						break;
				}
				if (HandShakeStatus==1)//退出消息队列
					break;
			}

			//DLE检查
			Peroid=0;HandShakeStatus=0;//发送态 握手IDLE
			if (!m_tcpClient.SendData(sendBuf,sendLen))//发送数据 7位
			{
				AfxMessageBox(_T("发送失败"));
				Peroid=1;//空置
				return false;
			}
			//释放内存
			delete[] sendBuf;
			sendBuf = NULL;		
			SetTimer(TIMERSEND,WAITSEND,NULL);//设置计时
			while (GetMessage(&msg,NULL,0,0L))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				switch (HandShakeStatus)
				{
					case -2:
						AfxMessageBox(_T("连接超时"));
						Peroid=1;
						KillTimer(TIMERSEND);
						return false;
					case -1:
						AfxMessageBox(_T("握手失败"));
						Peroid=1;
						KillTimer(TIMERSEND);
						return false;
					case 0:
						break;
					case 1:
						//AfxMessageBox(_T("发送完成"));
						Peroid=1;//发送态空置
						KillTimer(TIMERSEND);
						break;
					//return true;
				}
				if (HandShakeStatus==1)
					break;
			}	
			//一次发送完成

		}
		Sleep(WAITCONST);
		return true;
	}
	else
	{
		return false;	
	}
	
}
//接受字符串，机械手信息保存到RecvAStr，测距仪保存数据结果到fMResult
void CClientTCPDlg::RecvOrder()
{
	memset(RecvAStr,0,30);//接受数组清空
	Peroid=2;HandShakeStatus=12;//接受态 握手空状态
	MSG msg;
	if (m_tcpClient.Status3964R)//是否按照3964R协议
	{
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (HandShakeStatus==13)//接受完成
			{
				Peroid=1;
				if (LogStatus)
					fprintf_s(fErr, "收机械手数据完成\n");
				break;
			}
			else
			{
				if (HandShakeStatus==12)//空置
					continue;
				if (HandShakeStatus==-2)//OnTimer超时
				{
					AfxMessageBox(_T("收数据超时"));
					KillTimer(TIMERRECV);
					Peroid=1;
					HandShakeStatus=0;//空置
					if (LogStatus)
						fprintf_s(fErr,"Recv:收数据超时\n");
					break;
				}
				if (HandShakeStatus == -1)//接收0x15  不太可能出现
				{
					AfxMessageBox(_T("收数据NAK"));
					KillTimer(TIMERRECV);
					Peroid = 1;
					HandShakeStatus = 0;//空置
					if (LogStatus)
						fprintf_s(fErr, "Recv:收数据握手NAK\n");
					break;
				}
			}

		}
	}
	else
	{
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (DMeasureResult==1) 
			{
				CString str;
				str.Format(_T("测量成功%fm"),fMResult);
				AfxMessageBox(str);break;
			}
			if (DMeasureResult==-1) {AfxMessageBox(_T("测量失败"));break;}
		}
		AfxMessageBox(_T("测量完成"));
	}
}

//断开连接
bool CClientTCPDlg::RemoteServerDisconnect(void)
{
	if (m_tcpClient.Close())
	{
		if (LogStatus)
			fprintf_s(fErr,"端口已经断开\n");
		return true;
	}
	else
	{
		if (LogStatus)
			fprintf_s(fErr,"端口未能正常断开\n");
		AfxMessageBox(_T("连接断开失败"));
		return false;
	}	
	
}

//发送任意字符串
bool CClientTCPDlg::SendNetStr(char* SendAStr)
{
	if (m_tcpClient.Status3964R)
	{
		char * sendBuf;
		int sendLen=0;
		char CtrlString[2];
		MSG msg;
		char Bcc=0;
		int dBufSize=strlen(SendAStr);
		//分配目标缓存  
		sendBuf = new char[dBufSize+4];  
		memset(sendBuf, 0, dBufSize+4);  
		//转换  

		sprintf(sendBuf,"%s",SendAStr);
		sendBuf[dBufSize]=DLE;sendBuf[dBufSize+1]=ETX;

		Bcc=sendBuf[0];
		for (int i=1;i<dBufSize+2;i++)
		{
			Bcc=Bcc^sendBuf[i];
		}
		sendBuf[dBufSize+2]=Bcc;
		sendLen = strlen(sendBuf);   //暂时发%s貌似还没好好测试
		CtrlString[0]=STX;CtrlString[1]=NULL;
		Peroid=0;HandShakeStatus=0;//发送态  握手空置
		if (!m_tcpClient.SendData(CtrlString,1))//STX
		{
			AfxMessageBox(_T("发送失败"));
			if (LogStatus)
			{
				fprintf_s(fErr,"Send:STX发送失败\n");
			}
			Peroid=1;
			return false;
		}
		else
		{
			if (LogStatus)
			{
				PrintHexChar(CtrlString, 1, SENDSTATUS);
			}
		}
		SetTimer(TIMERSEND,WAITSEND,NULL);//设定计时
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			switch (HandShakeStatus)
			{
				case -2:
					AfxMessageBox(_T("连接超时"));
					if (LogStatus)
					{
						fprintf_s(fErr,"Recv:发送握手1超时\n");
					}
					Peroid=1;
					KillTimer(TIMERSEND);
					return false;
				case -1:
					AfxMessageBox(_T("握手1Nak失败"));
					if (LogStatus)
					{
						fprintf_s(fErr,"Recv:发送握手1Nak\n");
					}
					Peroid=1;
					KillTimer(TIMERSEND);
					return false;
				case 0:
					break;
				case 1:
					KillTimer(TIMERSEND);
					if (LogStatus)
					{
						fprintf_s(fErr,"发机械手握手1完成\n");
					}
					break;
			}
			if (HandShakeStatus==1)
				break;
		}

		//DLE检查
		Peroid=0;HandShakeStatus=0;
		if (!m_tcpClient.SendData(sendBuf,sendLen))
		{
			AfxMessageBox(_T("发送失败"));
			if (LogStatus)
			{
				fprintf_s(fErr,"Send:DataString发送失败\n");
			}
		}
		else
		{
			if (LogStatus)
			{
				PrintHexChar(sendBuf, sendLen, SENDSTATUS);
			}
		}
		//释放内存
		delete[] sendBuf;
		sendBuf = NULL;		
		SetTimer(TIMERSEND,WAITSEND,NULL);
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			switch (HandShakeStatus)
			{
				case -2:
					AfxMessageBox(_T("连接超时"));
					if (LogStatus)
					{
						fprintf_s(fErr,"Recv:发送握手2超时\n");
					}
					Peroid=1;
					KillTimer(TIMERSEND);
					return false;
				case -1:
					AfxMessageBox(_T("握手2Nak失败"));
					if (LogStatus)
					{
						fprintf_s(fErr,"Recv:发送握手2Nak\n");
					}
					Peroid=1;
					KillTimer(TIMERSEND);
					return false;
				case 0:
					break;
				case 1:
					AfxMessageBox(_T("发送完成"));//只处理发送一次
					Peroid=1;
					if (LogStatus)
					{
						fprintf_s(fErr,"发机械手握手2完成\n");
					}
					KillTimer(TIMERSEND);
					return true;
			}
		}	
	}
	else//激光测距通信  
	{
		if (!m_tcpClient.SendData(SendAStr,strlen(SendAStr)))
		{
			AfxMessageBox(_T("发送失败"));
			return false;
		}
		//释放内存

		MSG msg;
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (DMeasureResult==1) 
			{
				CString str;
				str.Format(_T("测量成功%fm"),fMResult);
				AfxMessageBox(str);break;
			}
			if (DMeasureResult==-1) {AfxMessageBox(_T("测量失败"));return false;}
		}
		AfxMessageBox(_T("测量完成"));
		return true;
	}
	return false;
	
}

void CClientTCPDlg::GetSystemTime(LPSTR str)
{
	time_t t=time(NULL);
	char tmp[64];
	strftime(tmp,sizeof(tmp),"%Y-%m-%d",localtime(&t));
	strcpy(str,LPCSTR(tmp));
}

void CClientTCPDlg::PrintHexChar(char* str,int len,int status)
{
	if (strlen(str)<len) len=strlen(str);
	if (status==SENDSTATUS)
		fprintf_s(fErr,"Send:");
	else
		fprintf_s(fErr,"Recv:");

	for (int i=0;i<len;i++)
	{
		fprintf_s(fErr,"%X ",str[i]);
	}
	fprintf_s(fErr,"\n");
}


//{
//	DWORD dBufSize=WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, NULL,0,NULL, FALSE);  
//	//分配目标缓存  
//	sendBuf = new char[dBufSize];  
//	memset(sendBuf, 0, dBufSize);  
//	//转换  
//	WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, sendBuf, dBufSize, NULL, FALSE);
//	sendLen = strlen(sendBuf)+1;
//	DMeasureResult=0;
//	//发送数据
//	if (!m_tcpClient.SendData(sendBuf,sendLen))
//	{
//		AfxMessageBox(_T("发送失败"));
//	}
//	//释放内存
//	delete[] sendBuf;
//	sendBuf = NULL;		
//	MSG msg;
//	while (GetMessage(&msg,NULL,0,0L))
//	{
//		TranslateMessage(&msg);
//		DispatchMessage(&msg);
//		if (DMeasureResult==1) 
//		{
//			CString str;
//			str.Format(_T("测量成功%fm"),fMResult);
//			AfxMessageBox(str);break;
//		}
//		if (DMeasureResult==-1) {AfxMessageBox(_T("测量失败"));break;}
//	}
//	AfxMessageBox(_T("测量完成"));
//}

//pEdtRecvMsg->GetWindowTextW(strOldRecv);
////
//int len=MultiByteToWideChar(CP_ACP,0,LPCSTR(pBuf),dwBufLen,NULL,0);
//wchar_t* pUnicode;
//pUnicode=new wchar_t[len+1];
//memset(pUnicode,0,(len+1)*sizeof(wchar_t));
//MultiByteToWideChar(CP_ACP,0,LPCSTR(pBuf),dwBufLen,pUnicode,len);
//strRecv.Empty();strRecv.Append(pUnicode);

////将新接收到的文本添加到接收框中
//strOldRecv.Append(pUnicode);

//strOldRecv = strOldRecv + L"\r\n";
//pEdtRecvMsg->SetWindowTextW(strOldRecv);

//释放内存
/*delete[] pUnicode;*/