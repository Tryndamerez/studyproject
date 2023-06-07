#include "RTSPServer.h"

int RTSPServer::Init(const std::string& strIP, short port)
{
    m_addr.Update(strIP, port);
    m_socket.Bind(m_addr);
    m_socket.Listen();
    return 0;
}

int RTSPServer::Invoke()
{
    m_threadMain.Start();
    return 0;
}

void RTSPServer::Stop()
{
    m_socket.Close();
    m_threadMain.Stop();
    m_pool.Stop();
}

RTSPServer::~RTSPServer()
{
    Stop();
}

int RTSPServer::threadWorker()
{
    EAddress client_addr;
    ESocket client = m_socket.Accept(client_addr);
    if (client != INVALID_SOCKET) {
        m_clients.PushBack(client);
        m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::ThreadSession));
    }
    
    return 0;
}

RTSPRequest RTSPServer::AnalyseRequest(const std::string& data)
{
    return RTSPRequest();
}

RTSPReply RTSPServer::MakeReply(const RTSPRequest& request)
{
    return RTSPReply();
}

int RTSPServer::ThreadSession()
{ 
    //TOOD：接受数据请求、解析请求、应答请求
    //TOOD 传参
    ESocket client;//TOOD 能否拿到
    EBuffer buffer(1024*16);
    int len = client.Recv(buffer);
    if (len <= 0) {
        //TOOD 清理掉client
        return -1;
    }
    buffer.resize(len);
    RTSPRequest req = AnalyseRequest(buffer);
    RTSPReply ack = MakeReply(req);
    client.Send(ack.toBuffer());
    return 0;
}
