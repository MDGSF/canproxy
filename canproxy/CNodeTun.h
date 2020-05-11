#pragma once

#include "CNode.h"
#include "tap.h"

class CNodeTun : public CNode
{
public:
	int m_iStart();
	int m_iRead(char* pcBuf, int iLen);
	int m_iWrite(const char* pcBuf, int iLen);
	int m_iStop();

private:
	TAPHandle m_handle;
};
