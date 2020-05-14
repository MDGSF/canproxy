#pragma once

class CNode
{
public:
	virtual const char* m_pcName() = 0;
	virtual int m_iStart() = 0;
	virtual int m_iRead(char* pcBuf, int iLen) = 0;
	virtual int m_iWrite(const char* pcBuf, int iLen) = 0;
	virtual int m_iStop() = 0;
};
