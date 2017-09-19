
// ClientTCPDlg.h : 头文件
//

#pragma once
#include "TCPClient.h"

// CClientTCPDlg 对话框
class CClientTCPDlg : public CDialogEx
{
// 构造
public:
	CClientTCPDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CLIENTTCP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CTCPClient m_tcpClient;//网口通信类
	static void CALLBACK OnDisConnect(void* pOwner);//断开连接 回调
	static void CALLBACK OnRead(void* pOwner,const char *buf,DWORD dwBufLen);//读函数 回调
	static void CALLBACK OnError(void* pOwner,int nErrorCode);//网口错误 回调
	afx_msg LONG OnRecvTCPData(WPARAM wParam,LPARAM lParam);//接受处理消息函数 重要
	afx_msg LONG OnClientDisconnect(WPARAM wParam,LPARAM lParam);//断开连接消息处理 可以删掉

public:
	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnBnClickedBtnDisconnect();
	afx_msg void OnBnClickedBtnSenddata();
	afx_msg void OnBnClickedBtnSet();
	
	afx_msg void OnBnClickedCheck3964r();
	DWORD m_remotePort;
	CString m_remoteHost;
	CString GetLocalIP();


	afx_msg void OnBnClickedButton2();//示例

	int DistanceQuery(LPCTSTR Hostname,float* dist);
	
	int DMeasureResult;
	float fMResult;//测距仪相关的两个数
	
	int HandShakeStatus;//3964R状态
	int Peroid;//收发状态
	char RecvAStr[30];//接收的字符串
	static const int LogStatus;
	static const int RetryStatus;
	void OnTimer(UINT_PTR nIDEvent);//定时器控制
	void GetSystemTime(LPSTR str);
	void PrintHexChar(char* str,int len,int status);//
	
	bool RemoteServerConnect(CString Hostname, DWORD Portname);//远程服务器连接
	bool SendNetNum(int* deltaDist, int numOfInt);//发整数数组
	void RecvOrder();//接受
	bool RemoteServerDisconnect(void);//远程服务器断开连接
	bool SendNetStr(char* SendAStr);//发字符串
	
	FILE* fErr;
};