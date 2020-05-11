#include <stdio.h>
#include "CNodeTun.h"

int CNodeTun::m_iStart()
{
	m_handle = iTapOpen("192.168.0.100", "255.255.255.0");
	if (m_handle == NULL)
	{
		return -1;
	}
	return 0;
}

int CNodeTun::m_iRead(char* pcBuf, int iLen)
{
	return iTapRead(m_handle, pcBuf, iLen);
}

int CNodeTun::m_iWrite(const char* pcBuf, int iLen)
{
	return iTapWrite(m_handle, pcBuf, iLen);
}

int CNodeTun::m_iStop()
{
	vTapClose(m_handle);
	return 0;
}