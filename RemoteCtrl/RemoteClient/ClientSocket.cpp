#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;

CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();

std::string GetErrInfo(int wasErrCode)
{
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, wasErrCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}

void Dump(BYTE* pData, size_t nSize)
{
	std::string strOut;
	for (size_t i = 0; i < nSize; i++)
	{
		char buf[8] = "";
		if (i > 0 && (i % 16 == 0)) strOut += "\n";
		snprintf(buf, sizeof(buf), "%02X", pData[i] & 0xFF);
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());
}

bool CClientSocket::Initsocket()
{
	if (m_sock != INVALID_SOCKET) CloseSocket();
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sock == -1) return false;
	sockaddr_in serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	TRACE("addr %08X nIP %08X\r\n", inet_addr("192.168.153.1"), m_nIP);
	serv_adr.sin_addr.s_addr = htonl(m_nIP);
	serv_adr.sin_port = htons(m_nPort);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE)
	{
		AfxMessageBox("指定IP地址不存在");
		return false;
	}
	int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
	if (ret == -1)
	{
		AfxMessageBox("连接失败");
		TRACE("连接失败:%d %s\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()).c_str());
		return false;
	}
	return true;
}

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed)
{
	if ((m_sock == INVALID_SOCKET) && (m_hThread == INVALID_HANDLE_VALUE))
	{
		//if (Initsocket() == false) return false;
		m_hThread = (HANDLE)_beginthread(CClientSocket::threadEntry, 0, this);
	}
	m_lock.lock();
	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent,lstPacks));
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));
	m_lstSend.push_back(pack);
	m_lock.unlock();
	WaitForSingleObject(pack.hEvent, INFINITE);
	std::map<HANDLE, std::list<CPacket>&> ::iterator it;
	it = m_mapAck.find(pack.hEvent);
	if (it != m_mapAck.end())
	{
		m_lock.lock();
		m_mapAck.erase(it);
		m_lock.lock();
		return true;
	}
	return false;
}

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();

}

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	Initsocket();
	while (m_sock != INVALID_SOCKET)
	{
		if (m_lstSend.size() > 0)
		{
			TRACE("lstSend size:%d\r\n", m_lstSend.size());
			m_lock.lock();
			CPacket& head = m_lstSend.front();
			m_lock.unlock();
			if (Send(head) == false)
			{
				TRACE("发送失败！\r\n");
				continue;
			}
			std::map<HANDLE, std::list<CPacket>&> ::iterator it;
			it = m_mapAck.find(head.hEvent); 
			if (it != m_mapAck.end())
			{
				std::map<HANDLE, bool> ::iterator it0 = m_mapAutoClosed.find(head.hEvent);
				do
				{
					int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
					if ((length > 0 )|| (index > 0))
					{
						index += length;
						size_t size = (size_t)index;
						CPacket pack((BYTE*)pBuffer, size);
						if (size > 0)
						{
							pack.hEvent = head.hEvent;
							it->second.push_back(pack);
							memmove(pBuffer, pBuffer + size, index - size);
							index -= size;
							if (it0->second == true)
							{
								SetEvent(head.hEvent);
							}
						}
						continue;
					}
					else if (length <= 0 && index <= 0)
					{
						CloseSocket();
						SetEvent(head.hEvent);//等到服务器关闭命令之后，再通知事情完成
						m_mapAutoClosed.erase(it0);
						break;
					}
				} while (it0->second == false || index > 0);
			}
			m_lock.lock();
			m_lstSend.pop_front();
			m_lock.unlock();
			if (Initsocket() == false)
			{
				Initsocket();
			}
		}
		Sleep(1);
	}
	CloseSocket();
}

bool CClientSocket::Send(const CPacket& pack)
{
	TRACE("m_sock:%d\r\n", m_sock);
	if (m_sock == -1) return false;
	std::string strOut;
	pack.Data(strOut);
	return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
}
