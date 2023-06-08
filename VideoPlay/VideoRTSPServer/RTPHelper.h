#pragma once
#include "base.h"
class RTPHeader
{
public:
	unsigned short csrccount : 4;
	unsigned short extension : 1;
	unsigned short padding : 1;
	unsigned short version : 2;//Œª”Ú
	unsigned short pytype : 7;
	unsigned short mark : 1;
	unsigned short serial;
	unsigned timestamp;
	unsigned ssrc;
	unsigned csrc[15];
public:
	//RTPHeader();
	//operator EBuffer();
};

class RTPFrame
{
public:
	RTPHeader m_head;
	EBuffer m_pyload;
};




class RTPHelper
{
public:
	RTPHelper();
	~RTPHelper();
	int SendMedialFrame(EBuffer& frame);
private:

};

