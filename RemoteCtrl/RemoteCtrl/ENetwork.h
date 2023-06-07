#pragma once
#include "ESocket.h"
#include "EdoyunThread.h"

class ENetwork
{

};

typedef int (*AcceptFunc)(void* arg,ESOCKET& client);
typedef int (*RecvFunc)(void* arg, const EBuffer& buffer);
typedef int (*SendFunc)(void* arg,ESOCKET& client,int ret);
typedef int (*RecvFromFunc)(void* arg, const EBuffer& buffer, ESockaddrIn& addr);
typedef int (*SendToFunc)(void* arg, const ESockaddrIn& addr,int ret);

class EServerParamter
{
public:
	EServerParamter(
		const std::string& ip = "0.0.0.0",
		short port = 9527, ETYPE type = ETYPE::ETypeTCP,
		AcceptFunc acceptf = NULL,
		RecvFunc recvf = NULL,
		SendFunc sendf = NULL,
		RecvFromFunc recvfromf = NULL,
		SendToFunc sendtof=NULL
	);
	//输入
	EServerParamter& operator<<(AcceptFunc func);
	EServerParamter& operator<<(RecvFunc func);
	EServerParamter& operator<<(SendFunc func);
	EServerParamter& operator<<(RecvFromFunc func);
	EServerParamter& operator<<(SendToFunc func);
	EServerParamter& operator<<(const std::string& ip);
	EServerParamter& operator<<(short port);
	EServerParamter& operator<<(ETYPE type);
	//输出
	EServerParamter& operator>>(AcceptFunc& func);
	EServerParamter& operator>>(RecvFunc& func);
	EServerParamter& operator>>(SendFunc& func);
	EServerParamter& operator>>(RecvFromFunc& func);
	EServerParamter& operator>>(SendToFunc& func);
	EServerParamter& operator>>(std::string& ip);
	EServerParamter& operator>>(short& port);
	EServerParamter& operator>>(ETYPE& type);
	//复制构造函数和=号承载 用于同类型赋值
	EServerParamter(const EServerParamter& param);
	EServerParamter operator = (const EServerParamter & param);
	std::string m_ip;
	short m_port;
	ETYPE m_type;
	AcceptFunc m_accept;
	RecvFunc m_recv;
	SendFunc m_send;
	RecvFromFunc m_recvfrom;
	SendToFunc m_sendto;
};

class EServer :public ThreadFuncBase
{
public:
	EServer(const EServerParamter& param);//何时设置关键参数，是需要依据个人开发经验和实际需求去调整
	~EServer();
	int Invoke(void* arg);
	int Send(ESOCKET& client,const EBuffer& buffer);
	int Sendto(ESockaddrIn& addr,const EBuffer& buffer);
	int Stop();
private:
	int threadFunc();
	int threadUDPFunc();
	int threadTCPFunc();
private:
	EServerParamter m_params;
	void* m_args;
	EdoyunThread m_thread;
	ESOCKET m_sock;
	std::atomic<bool> m_stop;
};
