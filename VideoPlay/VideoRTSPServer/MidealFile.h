#pragma once
#include "base.h"
class MidealFile
{
public:
	MidealFile();
	~MidealFile();
	int Open(const EBuffer& path, int nType = 96);
	//���buffer��sizeΪ0�����ʾû��֡��
	EBuffer ReadOneFrame();
	void Close();
	//���ú�ReadOneFrame�ֻ���ֵ����
	void Reset();
private:
	//����-1 ��ʾ����ʧ��
	long FindH264Head();
	EBuffer ReadH264Frame();
private:
	long m_size;
	FILE* m_file;
	EBuffer m_filepath;
	//96 H264
	int m_type;
};

