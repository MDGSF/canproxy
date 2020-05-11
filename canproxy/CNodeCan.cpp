#include "CNodeCan.h"
#include "log.h"
#include "can.h"

int CNodeCan::m_iStart()
{
	return iCanStart();
}

int CNodeCan::m_iRead(char* pcBuf, int iLen)
{
	return iCanRead(pcBuf, iLen);
}

int CNodeCan::m_iWrite(const char* pcBuf, int iLen)
{
	return iCanWrite(pcBuf, iLen);
}

int CNodeCan::m_iStop()
{
	vCanStop();
	return 0;
}