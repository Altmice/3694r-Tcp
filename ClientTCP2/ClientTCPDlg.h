
// ClientTCPDlg.h : ͷ�ļ�
//

#pragma once
#include "TCPClient.h"

// CClientTCPDlg �Ի���
class CClientTCPDlg : public CDialogEx
{
// ����
public:
	CClientTCPDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CLIENTTCP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CTCPClient m_tcpClient;//����ͨ����
	static void CALLBACK OnDisConnect(void* pOwner);//�Ͽ����� �ص�
	static void CALLBACK OnRead(void* pOwner,const char *buf,DWORD dwBufLen);//������ �ص�
	static void CALLBACK OnError(void* pOwner,int nErrorCode);//���ڴ��� �ص�
	afx_msg LONG OnRecvTCPData(WPARAM wParam,LPARAM lParam);//���ܴ�����Ϣ���� ��Ҫ
	afx_msg LONG OnClientDisconnect(WPARAM wParam,LPARAM lParam);//�Ͽ�������Ϣ���� ����ɾ��

public:
	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnBnClickedBtnDisconnect();
	afx_msg void OnBnClickedBtnSenddata();
	afx_msg void OnBnClickedBtnSet();
	
	afx_msg void OnBnClickedCheck3964r();
	DWORD m_remotePort;
	CString m_remoteHost;
	CString GetLocalIP();


	afx_msg void OnBnClickedButton2();//ʾ��

	int DistanceQuery(LPCTSTR Hostname,float* dist);
	
	int DMeasureResult;
	float fMResult;//�������ص�������
	
	int HandShakeStatus;//3964R״̬
	int Peroid;//�շ�״̬
	char RecvAStr[30];//���յ��ַ���
	static const int LogStatus;
	static const int RetryStatus;
	void OnTimer(UINT_PTR nIDEvent);//��ʱ������
	void GetSystemTime(LPSTR str);
	void PrintHexChar(char* str,int len,int status);//
	
	bool RemoteServerConnect(CString Hostname, DWORD Portname);//Զ�̷���������
	bool SendNetNum(int* deltaDist, int numOfInt);//����������
	void RecvOrder();//����
	bool RemoteServerDisconnect(void);//Զ�̷������Ͽ�����
	bool SendNetStr(char* SendAStr);//���ַ���
	
	FILE* fErr;
};