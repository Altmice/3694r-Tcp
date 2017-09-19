
// ClientTCPDlg.cpp : ʵ���ļ�
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

//����TCP �ͻ��˽��յ�������Ϣ
#define WM_RECV_TCP_DATA WM_USER + 101
//����TCP�ͻ������ӶϿ���Ϣ
#define WM_TCP_CLIENT_DISCONNECT WM_USER + 102
// CClientTCPDlg �Ի���

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


// CClientTCPDlg ��Ϣ�������

BOOL CClientTCPDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_remoteHost=GetLocalIP();
	m_remotePort=23;
	UpdateData(FALSE);
	CButton *Button=(CButton*)GetDlgItem(IDC_BTN_DISCONNECT);
	Button->EnableWindow(false);
	Button=(CButton*)GetDlgItem(IDC_BTN_SENDDATA);
	Button->EnableWindow(false);
	CButton *CheckBox=(CButton*)GetDlgItem(IDC_CHECK3964R);
	CheckBox->SetCheck(1);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CClientTCPDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CClientTCPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//���ӶϿ��¼�
void CALLBACK CClientTCPDlg::OnDisConnect(void* pOwner)
{
	//�õ�������ָ��
	CClientTCPDlg* pThis = (CClientTCPDlg*)pOwner;
	//������Ϣ��ʾ�ͻ������ӶϿ�
	pThis->PostMessage(WM_TCP_CLIENT_DISCONNECT,0,0);
}

//���ݽ����¼�
void CALLBACK CClientTCPDlg::OnRead(void* pOwner,const char * buf,DWORD dwBufLen )
{
	BYTE *pRecvBuf = NULL; //���ջ�����
	//�õ�������ָ��
	CClientTCPDlg* pThis = (CClientTCPDlg*)pOwner;
	//�����յĻ�����������pRecvBuf��
	pRecvBuf = new BYTE[dwBufLen];
	CopyMemory(pRecvBuf,buf,dwBufLen);

	//�����첽��Ϣ����ʾ�յ�TCP���ݣ���Ϣ�����꣬Ӧ�ͷ��ڴ�
	pThis->PostMessage(WM_RECV_TCP_DATA,WPARAM(pRecvBuf),dwBufLen);

}

//Socket�����¼�
void CALLBACK CClientTCPDlg::OnError(void* pOwner,int nErrorCode)
{
	TRACE(L"�ͻ���socket��������");
}

//TCP�������ݴ�����
//Peroid 0�� 2�� 1����
//HandShakeStatus ��״̬1���������� ��״̬11��13��������������
LONG CClientTCPDlg::OnRecvTCPData(WPARAM wParam,LPARAM lParam)
{
	CString strOldRecv = L"";//�϶Ի����ַ� ������
	CString strRecv = L"";//����CString�Ĵ���ʽ
	//���յ���BUF
	CHAR *pBuf = (CHAR*)wParam;
	//���յ���BUF����
	DWORD dwBufLen = lParam;
	//���տ�
	//CEdit *pEdtRecvMsg = (CEdit*)GetDlgItem(IDC_EDIT_RECEIVE);
	//ASSERT(pEdtRecvMsg != NULL);

	if (m_tcpClient.Status3964R)
	{
		char tmp;
		switch (Peroid)
		{
			case 0://����
				
				tmp=0;
				tmp=pBuf[0];
				switch (tmp)
				{
					case DLE://��������
						HandShakeStatus=1;
						if (LogStatus)
						{
							PrintHexChar(pBuf,1,RECVSTATUS);
						}
						break;
					case NAK://NAK��
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
			case 2://����
			
				tmp=0;
				tmp=pBuf[0];
				char CtrlString[4];
				CtrlString[0]=DLE;CtrlString[1]=NULL;
				switch (tmp)
				{
					case STX:

						if (!m_tcpClient.SendData(CtrlString,1))
						{
							AfxMessageBox(_T("�ظ�STXʧ��"));
							if (LogStatus)
							{
								fprintf_s(fErr,"Send:�ظ�STXʧ��\n");
							}
							Peroid=1;
							return 0;
						}
						if (LogStatus)
						{
							PrintHexChar(pBuf,1,RECVSTATUS);
							PrintHexChar(CtrlString,1,SENDSTATUS);
							fprintf_s(fErr,"�ջ�е������1���\n");
						}
						HandShakeStatus=11;//����1Ok
						SetTimer(TIMERRECV,WAITRECV,NULL);//������Ӧʱ��
						break;
					case NAK:
						AfxMessageBox(_T("ͨ��NAK"));
						if (LogStatus)
						{
							fprintf_s(fErr,"Recv:����ͨ��NAK\n");
							PrintHexChar(pBuf,1,RECVSTATUS);
						}
						HandShakeStatus=-1;
						break;
					
					default:
						if (HandShakeStatus==11)
						{
							if (!m_tcpClient.SendData(CtrlString,1))//����2���
							{
								AfxMessageBox(_T("�ظ�DataStringʧ��"));
								if (LogStatus)
								{
									fprintf_s(fErr,"Send:�ظ�DataStringʧ��\n");
								}
								Peroid=1;
								break;
							}
							KillTimer(TIMERRECV);//�ص����ݼ�ʱ
							HandShakeStatus=13;
							strcpy(RecvAStr,pBuf);//ȡ����
							RecvAStr[7] = 0x00;
							if (LogStatus)
							{
								PrintHexChar(RecvAStr,strlen(RecvAStr),RECVSTATUS);
								PrintHexChar(CtrlString,1,SENDSTATUS);
								fprintf_s(fErr,"�ջ�е������2���\n");
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
		////�õ����տ��е���ʷ�ı�
		//pEdtRecvMsg->GetWindowTextW(strOldRecv);
		//
		int len=MultiByteToWideChar(CP_ACP,0,LPCSTR(pBuf),dwBufLen,NULL,0);
		wchar_t* pUnicode;
		pUnicode=new wchar_t[len+1];
		memset(pUnicode,0,(len+1)*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP,0,LPCSTR(pBuf),dwBufLen,pUnicode,len);
		strRecv.Empty();strRecv.Append(pUnicode);

		//���½��յ����ı���ӵ����տ���
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

		//�ͷ��ڴ�
		delete[] pBuf;
		delete[] pUnicode;
		pBuf = NULL;
	}
	
	return 0;
}

//�ͻ������ӶϿ���Ϣ����
LONG CClientTCPDlg::OnClientDisconnect(WPARAM wParam,LPARAM lParam)
{
	//�õ�״̬����ǩ
	CStatic * pStatus = (CStatic *)GetDlgItem(IDC_CONNECTSTATUS);
	ASSERT(pStatus != NULL);

	pStatus->SetWindowText(_T("���ӶϿ�"));
	return 0;
}

//�õ����ص�IP��ַ
CString CClientTCPDlg::GetLocalIP()
{
	HOSTENT *LocalAddress;
	char	*Buff;
	TCHAR	*wBuff;
	CString strReturn = _T("");

	//�����µĻ�����
	Buff = new char[256];
	wBuff = new TCHAR[256];
	//�ÿջ�����
	memset(Buff, '\0', 256);
	memset(wBuff, TEXT('\0'), 256*sizeof(TCHAR));
	//�õ����ؼ������
	if (gethostname(Buff, 256) == 0)
	{
		//ת����˫�ֽ��ַ���
		mbstowcs(wBuff, Buff, 256);
		//�õ����ص�ַ
		LocalAddress = gethostbyname(Buff);
		//�ÿ�buff
		memset(Buff, '\0', 256);
		//��ϱ���IP��ַ
		sprintf(Buff, "%d.%d.%d.%d\0", LocalAddress->h_addr_list[0][0] & 0xFF,
			LocalAddress->h_addr_list[0][1] & 0x00FF, LocalAddress->h_addr_list[0][2] & 0x0000FF, LocalAddress->h_addr_list[0][3] & 0x000000FF);
		//�ÿ�wBuff
		memset(wBuff, TEXT('\0'), 256*sizeof(TCHAR));
		//ת����˫�ֽ��ַ���
		mbstowcs(wBuff, Buff, 256);
		//���÷���ֵ
		strReturn = wBuff;
	}
	else
	{
	}

	//�ͷ�Buff������
	delete[] Buff;
	Buff = NULL;
	//�ͷ�wBuff������
	delete[] wBuff;
	wBuff = NULL;
	return strReturn;
}


void CClientTCPDlg::OnBnClickedBtnConnect()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CStatic *pStatus = (CStatic*)GetDlgItem(IDC_CONNECTSTATUS);
	ASSERT(pStatus != NULL);
	//����m_tcpClient����
	m_tcpClient.m_remoteHost = m_remoteHost;
	m_tcpClient.m_port = m_remotePort;
	m_tcpClient.OnDisConnect = OnDisConnect;
	m_tcpClient.OnRead = OnRead;
	m_tcpClient.OnError = OnError;
	//�򿪿ͻ���socket
	m_tcpClient.Open(this);

	//�����������������
	if (m_tcpClient.Connect())
	{

		pStatus->SetWindowText(L"��������");
		UpdateData(FALSE);
	}
	else
	{
		AfxMessageBox(_T("��������ʧ��"));
		pStatus->SetWindowText(L"���ӶϿ�");
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CStatic *pStatus = (CStatic*)GetDlgItem(IDC_CONNECTSTATUS);
	ASSERT(pStatus != NULL);
	//�رտͻ����׽���
	if (m_tcpClient.Close())
	{
		pStatus->SetWindowText(L"���ӶϿ�");
	}
	else
	{
		AfxMessageBox(_T("���ӶϿ�ʧ��"));
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strSendData;
	char * sendBuf;
	int sendLen=0;
	CEdit *pEdtSend = (CEdit*)GetDlgItem(IDC_EDIT_SEND);
	pEdtSend->GetWindowTextW(strSendData);

	//���÷��ͻ�����
//	sendLen = strSendData.GetLength()*2 + 2;
//	sendBuf = new char[sendLen];
//	ZeroMemory(sendBuf,sendLen);
//	wcstombs(sendBuf,strSendData,sendLen);
	
	//�������͵��ַ���ת���ɵ��ֽڣ����з���
	//sendBuf = new char[strSendData.GetLength()*2+1];
	//ZeroMemory(sendBuf,strSendData.GetLength()*2+1);
	//ת���ɵ��ֽڽ��з���	
	//WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,strSendData.GetBuffer(strSendData.GetLength())
	//   ,strSendData.GetLength(),sendBuf,strSendData.GetLength()*2,NULL,NULL);
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK3964R))
	{
		char CtrlString[4];
		MSG msg;
		char Bcc=0;
		DWORD dBufSize=WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, NULL,0,NULL, FALSE);  
		//����Ŀ�껺��  
		sendBuf = new char[dBufSize+3];  
		memset(sendBuf, 0, dBufSize+3);  
		//ת��  
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
			AfxMessageBox(_T("����ʧ��"));
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
					AfxMessageBox(_T("���ӳ�ʱ"));
					Peroid=1;
					KillTimer(TIMERSEND);
					return;
				case -1:
					AfxMessageBox(_T("����Nakʧ��"));
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

		//DLE���
		Peroid=0;HandShakeStatus=0;
		if (!m_tcpClient.SendData(sendBuf,sendLen))
		{
			AfxMessageBox(_T("����ʧ��"));
		}
		//�ͷ��ڴ�
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
					AfxMessageBox(_T("���ӳ�ʱ"));
					Peroid=1;
					KillTimer(TIMERSEND);
					return;
				case -1:
					AfxMessageBox(_T("����ʧ��"));
					Peroid=1;
					KillTimer(TIMERSEND);
					return;
				case 0:
					break;
				case 1:
					AfxMessageBox(_T("�������"));
					Peroid=1;
					KillTimer(TIMERSEND);
					return ;
			}
		}	
	}
	else
	{
		DWORD dBufSize=WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, NULL,0,NULL, FALSE);  
		//����Ŀ�껺��  
		sendBuf = new char[dBufSize];  
		memset(sendBuf, 0, dBufSize);  
	 //ת��  
		WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, sendBuf, dBufSize, NULL, FALSE);
		sendLen = strlen(sendBuf)+1;
		DMeasureResult=0;
		//��������
		if (!m_tcpClient.SendData(sendBuf,sendLen))
		{
			AfxMessageBox(_T("����ʧ��"));
		}
		//�ͷ��ڴ�
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
				str.Format(_T("�����ɹ�%fm"),fMResult);
				AfxMessageBox(str);break;
			}
			if (DMeasureResult==-1) {AfxMessageBox(_T("����ʧ��"));break;}
		}
		AfxMessageBox(_T("�������"));
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
		//�򿪿ͻ���socket
		m_tcpClient.Open(this);

		//�����������������
		if (m_tcpClient.Connect())
		{
			CString strSendData;
			char * sendBuf;
			int sendLen=0;
			strSendData.Format(_T("D"));
			DWORD dBufSize=WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, NULL,0,NULL, FALSE);  
			//����Ŀ�껺��  
			sendBuf = new char[dBufSize];  
			memset(sendBuf, 0, dBufSize);  
		//ת��  
			WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, sendBuf, dBufSize, NULL, FALSE);
			sendLen = strlen(sendBuf)+1;
			DMeasureResult=0;
			//��������
			if (!m_tcpClient.SendData(sendBuf,sendLen))
			{
				AfxMessageBox(_T("����ʧ��"));
				return -1;
			}
			//�ͷ��ڴ�
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
					str.Format(_T("�����ɹ�%fm"),fMResult);
					*dist=fMResult;
					dist++;
					AfxMessageBox(str);break;
				}
				if (DMeasureResult==-1) {AfxMessageBox(_T("����ʧ��"));break;}
				if (DMeasureResult==-2) {AfxMessageBox(_T("������ʱ�����ͨ��"));break;}
			}
			AfxMessageBox(_T("�������"));
		}
		else
		{
			AfxMessageBox(_T("��������ʧ��"));
			return -1;
		}
		KillTimer(TIMERDIST);
		if (m_tcpClient.Close())
		{
		
		}
		else
		{
			AfxMessageBox(_T("���ӶϿ�ʧ��"));
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
			//AfxMessageBox(_T("�����ݳ�ʱ"));
			//KillTimer(TIMERRECV);
			HandShakeStatus=-2;
			break;
	}
}
void CClientTCPDlg::OnBnClickedBtnSet()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	m_remoteHost=GetLocalIP();
	//m_remotePort=23;
	UpdateData(FALSE);
}


void CClientTCPDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		fprintf_s(fErr,"%s  �������̿�ʼ\n",dateStr);
		
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
	//		AfxMessageBox(_T("���Զ�ξ����ܷ���"));
	//		if (LogStatus)
	//			fprintf_s(fErr,"���Զ�ξ�����ʧ��\n");
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
				AfxMessageBox(_T("���Զ�ξ����ܷ���"));
				if (LogStatus)
					fprintf_s(fErr, "���Զ�ξ�����ʧ��\n");
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
		fprintf_s(fErr, "%s  �������̽���\n", dateStr);
		fclose(fErr);
	}
}


void CClientTCPDlg::OnBnClickedCheck3964r()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK3964R))
	{
		AfxMessageBox(_T("�б��"));
		m_tcpClient.Status3964R=true;
	}
	else
	{
		m_tcpClient.Status3964R=false;
		AfxMessageBox(_T("�ޱ��"));
	}
}

//IP��ַHostname �˿ں�Portname
bool CClientTCPDlg::RemoteServerConnect(CString Hostname, DWORD Portname)
{
	//����m_tcpClient����
	m_tcpClient.m_remoteHost = Hostname;
	m_tcpClient.m_port = Portname;
	m_tcpClient.OnDisConnect = OnDisConnect;
	m_tcpClient.OnRead = OnRead;
	m_tcpClient.OnError = OnError;
	//�򿪿ͻ���socket
	m_tcpClient.Open(this);

	//�����������������
	if (m_tcpClient.Connect())
	{
		if (LogStatus)
			fprintf_s(fErr,"�˿�%d���ӳɹ�\n",Portname);
		return true;		
	}
	else
	{
		if (LogStatus)
			fprintf_s(fErr,"�˿�%d����ʧ��\n",Portname);
		AfxMessageBox(_T("��������ʧ��"));
		return false;
	}

}

//����ͷָ�� ���͵����ָ��� ����ÿ������ռ4���ֽ�
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
			//����Ŀ�껺��  
			sendBuf = new char[dBufSize+4];  
			memset(sendBuf, 0, dBufSize+4);  
			//ת��  
			sprintf(sendBuf,"%04d",abs(deltaDist[k]));
			sendBuf[dBufSize]=DLE;sendBuf[dBufSize+1]=ETX;
			if (abs(deltaDist[k])>1200)
			{
				AfxMessageBox(_T("��ֵ��������"));
				Peroid=1;//��ֹ״̬
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
			Peroid=0;HandShakeStatus=0;//����̬ ����IDLE
			if (!m_tcpClient.SendData(CtrlString,1))
			{
				AfxMessageBox(_T("����ʧ��"));
				Peroid=1;
				return false;
			}
			SetTimer(TIMERSEND,WAITSEND,NULL);//��0x10�ظ�
			while (GetMessage(&msg,NULL,0,0L))//������Ϣ���У��ȴ�����recv_tcp_data
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				switch (HandShakeStatus)
				{
					case -2:
						AfxMessageBox(_T("���ӳ�ʱ"));//��Ӧ��ʱ
						Peroid=1;//����
						KillTimer(TIMERSEND);//�ؼ�ʱ��
						return false;//���ش���
					case -1:
						AfxMessageBox(_T("����Nakʧ��"));//0x15
						Peroid=1;//����
						KillTimer(TIMERSEND);//�ؼ�ʱ��
						return false;//����
					case 0:
						break;//����
					case 1:
						KillTimer(TIMERSEND);//0x10
						break;
				}
				if (HandShakeStatus==1)//�˳���Ϣ����
					break;
			}

			//DLE���
			Peroid=0;HandShakeStatus=0;//����̬ ����IDLE
			if (!m_tcpClient.SendData(sendBuf,sendLen))//�������� 7λ
			{
				AfxMessageBox(_T("����ʧ��"));
				Peroid=1;//����
				return false;
			}
			//�ͷ��ڴ�
			delete[] sendBuf;
			sendBuf = NULL;		
			SetTimer(TIMERSEND,WAITSEND,NULL);//���ü�ʱ
			while (GetMessage(&msg,NULL,0,0L))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				switch (HandShakeStatus)
				{
					case -2:
						AfxMessageBox(_T("���ӳ�ʱ"));
						Peroid=1;
						KillTimer(TIMERSEND);
						return false;
					case -1:
						AfxMessageBox(_T("����ʧ��"));
						Peroid=1;
						KillTimer(TIMERSEND);
						return false;
					case 0:
						break;
					case 1:
						//AfxMessageBox(_T("�������"));
						Peroid=1;//����̬����
						KillTimer(TIMERSEND);
						break;
					//return true;
				}
				if (HandShakeStatus==1)
					break;
			}	
			//һ�η������

		}
		Sleep(WAITCONST);
		return true;
	}
	else
	{
		return false;	
	}
	
}
//�����ַ�������е����Ϣ���浽RecvAStr������Ǳ������ݽ����fMResult
void CClientTCPDlg::RecvOrder()
{
	memset(RecvAStr,0,30);//�����������
	Peroid=2;HandShakeStatus=12;//����̬ ���ֿ�״̬
	MSG msg;
	if (m_tcpClient.Status3964R)//�Ƿ���3964RЭ��
	{
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (HandShakeStatus==13)//�������
			{
				Peroid=1;
				if (LogStatus)
					fprintf_s(fErr, "�ջ�е���������\n");
				break;
			}
			else
			{
				if (HandShakeStatus==12)//����
					continue;
				if (HandShakeStatus==-2)//OnTimer��ʱ
				{
					AfxMessageBox(_T("�����ݳ�ʱ"));
					KillTimer(TIMERRECV);
					Peroid=1;
					HandShakeStatus=0;//����
					if (LogStatus)
						fprintf_s(fErr,"Recv:�����ݳ�ʱ\n");
					break;
				}
				if (HandShakeStatus == -1)//����0x15  ��̫���ܳ���
				{
					AfxMessageBox(_T("������NAK"));
					KillTimer(TIMERRECV);
					Peroid = 1;
					HandShakeStatus = 0;//����
					if (LogStatus)
						fprintf_s(fErr, "Recv:����������NAK\n");
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
				str.Format(_T("�����ɹ�%fm"),fMResult);
				AfxMessageBox(str);break;
			}
			if (DMeasureResult==-1) {AfxMessageBox(_T("����ʧ��"));break;}
		}
		AfxMessageBox(_T("�������"));
	}
}

//�Ͽ�����
bool CClientTCPDlg::RemoteServerDisconnect(void)
{
	if (m_tcpClient.Close())
	{
		if (LogStatus)
			fprintf_s(fErr,"�˿��Ѿ��Ͽ�\n");
		return true;
	}
	else
	{
		if (LogStatus)
			fprintf_s(fErr,"�˿�δ�������Ͽ�\n");
		AfxMessageBox(_T("���ӶϿ�ʧ��"));
		return false;
	}	
	
}

//���������ַ���
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
		//����Ŀ�껺��  
		sendBuf = new char[dBufSize+4];  
		memset(sendBuf, 0, dBufSize+4);  
		//ת��  

		sprintf(sendBuf,"%s",SendAStr);
		sendBuf[dBufSize]=DLE;sendBuf[dBufSize+1]=ETX;

		Bcc=sendBuf[0];
		for (int i=1;i<dBufSize+2;i++)
		{
			Bcc=Bcc^sendBuf[i];
		}
		sendBuf[dBufSize+2]=Bcc;
		sendLen = strlen(sendBuf);   //��ʱ��%sò�ƻ�û�úò���
		CtrlString[0]=STX;CtrlString[1]=NULL;
		Peroid=0;HandShakeStatus=0;//����̬  ���ֿ���
		if (!m_tcpClient.SendData(CtrlString,1))//STX
		{
			AfxMessageBox(_T("����ʧ��"));
			if (LogStatus)
			{
				fprintf_s(fErr,"Send:STX����ʧ��\n");
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
		SetTimer(TIMERSEND,WAITSEND,NULL);//�趨��ʱ
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			switch (HandShakeStatus)
			{
				case -2:
					AfxMessageBox(_T("���ӳ�ʱ"));
					if (LogStatus)
					{
						fprintf_s(fErr,"Recv:��������1��ʱ\n");
					}
					Peroid=1;
					KillTimer(TIMERSEND);
					return false;
				case -1:
					AfxMessageBox(_T("����1Nakʧ��"));
					if (LogStatus)
					{
						fprintf_s(fErr,"Recv:��������1Nak\n");
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
						fprintf_s(fErr,"����е������1���\n");
					}
					break;
			}
			if (HandShakeStatus==1)
				break;
		}

		//DLE���
		Peroid=0;HandShakeStatus=0;
		if (!m_tcpClient.SendData(sendBuf,sendLen))
		{
			AfxMessageBox(_T("����ʧ��"));
			if (LogStatus)
			{
				fprintf_s(fErr,"Send:DataString����ʧ��\n");
			}
		}
		else
		{
			if (LogStatus)
			{
				PrintHexChar(sendBuf, sendLen, SENDSTATUS);
			}
		}
		//�ͷ��ڴ�
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
					AfxMessageBox(_T("���ӳ�ʱ"));
					if (LogStatus)
					{
						fprintf_s(fErr,"Recv:��������2��ʱ\n");
					}
					Peroid=1;
					KillTimer(TIMERSEND);
					return false;
				case -1:
					AfxMessageBox(_T("����2Nakʧ��"));
					if (LogStatus)
					{
						fprintf_s(fErr,"Recv:��������2Nak\n");
					}
					Peroid=1;
					KillTimer(TIMERSEND);
					return false;
				case 0:
					break;
				case 1:
					AfxMessageBox(_T("�������"));//ֻ������һ��
					Peroid=1;
					if (LogStatus)
					{
						fprintf_s(fErr,"����е������2���\n");
					}
					KillTimer(TIMERSEND);
					return true;
			}
		}	
	}
	else//������ͨ��  
	{
		if (!m_tcpClient.SendData(SendAStr,strlen(SendAStr)))
		{
			AfxMessageBox(_T("����ʧ��"));
			return false;
		}
		//�ͷ��ڴ�

		MSG msg;
		while (GetMessage(&msg,NULL,0,0L))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (DMeasureResult==1) 
			{
				CString str;
				str.Format(_T("�����ɹ�%fm"),fMResult);
				AfxMessageBox(str);break;
			}
			if (DMeasureResult==-1) {AfxMessageBox(_T("����ʧ��"));return false;}
		}
		AfxMessageBox(_T("�������"));
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
//	//����Ŀ�껺��  
//	sendBuf = new char[dBufSize];  
//	memset(sendBuf, 0, dBufSize);  
//	//ת��  
//	WideCharToMultiByte(CP_ACP, 0, strSendData.GetBuffer(), -1, sendBuf, dBufSize, NULL, FALSE);
//	sendLen = strlen(sendBuf)+1;
//	DMeasureResult=0;
//	//��������
//	if (!m_tcpClient.SendData(sendBuf,sendLen))
//	{
//		AfxMessageBox(_T("����ʧ��"));
//	}
//	//�ͷ��ڴ�
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
//			str.Format(_T("�����ɹ�%fm"),fMResult);
//			AfxMessageBox(str);break;
//		}
//		if (DMeasureResult==-1) {AfxMessageBox(_T("����ʧ��"));break;}
//	}
//	AfxMessageBox(_T("�������"));
//}

//pEdtRecvMsg->GetWindowTextW(strOldRecv);
////
//int len=MultiByteToWideChar(CP_ACP,0,LPCSTR(pBuf),dwBufLen,NULL,0);
//wchar_t* pUnicode;
//pUnicode=new wchar_t[len+1];
//memset(pUnicode,0,(len+1)*sizeof(wchar_t));
//MultiByteToWideChar(CP_ACP,0,LPCSTR(pBuf),dwBufLen,pUnicode,len);
//strRecv.Empty();strRecv.Append(pUnicode);

////���½��յ����ı���ӵ����տ���
//strOldRecv.Append(pUnicode);

//strOldRecv = strOldRecv + L"\r\n";
//pEdtRecvMsg->SetWindowTextW(strOldRecv);

//�ͷ��ڴ�
/*delete[] pUnicode;*/