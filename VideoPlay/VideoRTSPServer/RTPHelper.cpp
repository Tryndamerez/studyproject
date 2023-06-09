#include "RTPHelper.h"
#include <Windows.h>
#define RTP_MAX_SIZE 1300

int RTPHelper::SendMedialFrame(RTPFrame& rtpframe, EBuffer& frame, const EAddress& client)
{
    size_t frame_size = frame.size();
    int sepsize = GetFrameSepSize(frame);
    frame_size -= sepsize;
    BYTE* pFrame = sepsize + (BYTE*)frame;
    if (frame_size > RTP_MAX_SIZE) {//分片
        BYTE nalu = pFrame[0] & 0x1F;
        size_t restsize = frame_size % RTP_MAX_SIZE;
        size_t count = frame_size / RTP_MAX_SIZE;
        for (size_t i = 0; i < count; i++) {
            rtpframe.m_pyload.resize(RTP_MAX_SIZE + 2);
            ((BYTE*)rtpframe.m_pyload)[0] = 0x60 | 28;//0110 0000 | 0001 1100
            ((BYTE*)rtpframe.m_pyload)[1] = nalu;//0000 0000 中间
            if (i == 0)
                ((BYTE*)rtpframe.m_pyload)[1] |= 0x80;//1000 0000 开始
            else if ((restsize == 0) && (i == count - 1))
                ((BYTE*)rtpframe.m_pyload)[1] |= 0x40;//0100 0000 结束
            memcpy(2 + (BYTE*)rtpframe.m_pyload, pFrame + RTP_MAX_SIZE * i + 1, RTP_MAX_SIZE);//+1表示跳过第一个字节
            SendFrame(rtpframe, client);
            rtpframe.m_head.serial++;
        }
        if (restsize > 0) {
            //处理尾巴 
            rtpframe.m_pyload.resize(restsize + 2);
            ((BYTE*)rtpframe.m_pyload)[0] = 0x60 | 28;//0110 0000 | 0001 1100
			((BYTE*)rtpframe.m_pyload)[1] = nalu;//0000 0000 中间
            ((BYTE*)rtpframe.m_pyload)[1] |= 0x40;//0100 0000 结束
            memcpy(2 + (BYTE*)rtpframe.m_pyload, pFrame + RTP_MAX_SIZE * count + 1, restsize);
            SendFrame(rtpframe, client);
            rtpframe.m_head.serial++;
        }
    }
    else {//小packet
        rtpframe.m_pyload.resize(frame.size() - sepsize);
        memcpy(rtpframe.m_pyload, pFrame, frame.size() - sepsize);
        SendFrame(rtpframe, client);
        //序列号是累加的，时间戳一般是计算出来的，从0开始，每帧追加 时间频率90000/每秒帧数24
        rtpframe.m_head.serial++;     
             
    }
    //时间戳使累加的，累加的量是时钟频率/每秒帧数 取整
    rtpframe.m_head.timestamp += 90000 / 24;
    //发送后加入休眠等待发送完成 控制发送速度
    Sleep(1000 / 30);
    return 0;
}

int RTPHelper::GetFrameSepSize(EBuffer& frame)
{
    BYTE buf[] = { 0,0,0,1 };
    if (memcmp(frame, buf, 4) == 0) return 4;
    return 3;
}

int RTPHelper::SendFrame(const EBuffer& frame, const EAddress& client)
{
    int ret = sendto(m_udp, frame, frame.size(), 0, client, client.Size());
    return 0;
}

RTPHeader::RTPHeader()
{
    csrccount = 0;
    extension = 0;
    padding = 0;
    version = 2;
    pytype = 96;
    mark = 0;
    serial = 0;
    timestamp = 0;
    ssrc = 0x98765432;
    memset(csrc, 0, sizeof(csrc));
}

RTPHeader::RTPHeader(const RTPHeader& header)
{
    memset(csrc, 0, sizeof(csrc));
    int size = 14 + 4 * csrccount;
    memcpy(this, &header, size);
}

RTPHeader& RTPHeader::operator=(const RTPHeader& header)
{
    if (this != &header) {
		int size = 14 + 4 * csrccount;
		memcpy(this, &header, size);
    }
    return *this;
}

RTPHeader::operator EBuffer()
{
    RTPHeader header = *this;
    header.serial = htons(header.serial);
    header.timestamp = htonl(header.timestamp);
    header.ssrc = htonl(header.ssrc);
    int size = 12 + 4 * csrccount;
    EBuffer result(size);
    memcpy(result, this, size);
    return result;
}

RTPFrame::operator EBuffer()
{
    EBuffer result;
    result += (EBuffer)m_head;
    result += m_pyload;
    return result;
}
