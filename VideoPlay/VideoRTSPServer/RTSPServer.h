#pragma once
#include "Socket.h"
#include "EdoyunThread.h"
#include "CEdoyunQueue.h"
#include <string>
#include <map>
class RTSPRequest
{
public:
	RTSPRequest();
	RTSPRequest(const RTSPRequest& protocol);
	RTSPRequest& operator=(const RTSPRequest& protocol);
	~RTSPRequest() { m_method = -1; }
	void SetMethod(const EBuffer& method);
	void SetUrl(const EBuffer& url);
	void SetSrquence(const EBuffer& seq);
	void SetClientPort(int ports[]);
	void SetSession(const EBuffer& session);
	int method() const { return m_method; }
	const EBuffer& url() const { return m_url; }
	const EBuffer& session()const { return m_session; }
	const EBuffer& sequence()const { return m_seq; }
	const EBuffer& port(int index = 0)const { return index ? m_client_port[1] : m_client_port[0];}
private:
	int m_method;//-1 初始化 0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 5 TEARDOWN
	EBuffer m_url;
	EBuffer m_session;
	EBuffer m_seq;
	EBuffer m_client_port[2];
};


class RTSPReply
{
public:
	RTSPReply();
	RTSPReply(const RTSPReply& protocol);
	RTSPReply& operator=(const RTSPReply& protocol);
	~RTSPReply(){}
	EBuffer toBuffer();
	void SetMethod(int method);
	void SetOptions(const EBuffer& options);
	void SetSequnce(const EBuffer& seq);
	void SetSdp(const EBuffer& sdp);
	void SetClientPort(const EBuffer& port0, const EBuffer& port1);
	void SetServerPort(const EBuffer& port0, const EBuffer& port1);
	void SetSession(const EBuffer& session);
private:
	int m_method;//0 OPTIONS 1 DESCRIBE 2 SETUP 3 PLAY 5 TEARDOWN
	int m_client_port[2];
	int m_server_port[2];
	EBuffer m_sdp;
	EBuffer m_options;
	EBuffer m_session;
	EBuffer m_seq;
};


class RTSPSession
{
public:
	RTSPSession();
	RTSPSession(const ESocket& client);
	RTSPSession(const RTSPSession& session);
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession(){}
	int PickRequestAndReply();
private:
	EBuffer PickOneLine(EBuffer& buffer);
	EBuffer Pick();
	RTSPRequest AnalysRequest(const EBuffer& buffer);
	RTSPReply Reply(const RTSPRequest& request);
private:
	EBuffer m_id;
	ESocket m_client;
};

class RTSPServer:public ThreadFuncBase
{
public:
	RTSPServer() 
		:m_socket(true),m_status(0) ,m_pool(10)
	{
		m_threadMain.UpdateWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::threadWorker));
	}
	int Init(const std::string& strIP = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
	~RTSPServer();
protected:
	//返回0继续 返回负数终止 返回其他警告
	int threadWorker();
	int ThreadSession();
private:
	static Socketiniter m_initer;
	ESocket m_socket;
	EAddress m_addr;
	int m_status;//0 未初始化 1初始化完成 2 正在运行 3关闭
	EdoyunThread m_threadMain;
	EdoyunThreadPool m_pool;
	CEdoyunQueue<RTSPSession> m_lstSession;
};

